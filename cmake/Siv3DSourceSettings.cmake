# Per-file compile settings of the CMake build. Each one is also recorded as a
# fact, which siv3d_finish_sources() writes out with the target's sources so
# that ctest can compare them with the Visual Studio projects
# (cmake/Siv3DCheckVcxproj.cmake).
#
#   siv3d_source_settings(<target> [CONFIG <Debug|Release>]
#       [WARNING_LEVEL <0-4>] [DISABLE_WARNINGS <number>...]
#       [DEFINITIONS <definition>...] [INCLUDE_DIRECTORIES <dir>...]
#       [NO_PRECOMPILED_HEADER]
#       FILES <path>... | DIRECTORY <dir>)
#
# Paths are relative to the repository root. DIRECTORY selects the target's
# sources under <dir>. Without CONFIG, a setting applies to both
# configurations.
#
#   siv3d_finish_sources(<target> WARNING_LEVEL <0-4> FACTS <file>)
#
# Gives the target its warning level and writes the facts file. Without
# per-file levels the level is a target option, which also covers the
# precompiled header CMake generates. With them, every file gets an explicit
# level, its own or the target's, since a per-file level after a target-wide
# one draws D9025; such a target cannot have a precompiled header, which would
# get no level (C4652).

function(siv3d_source_settings _target)
    cmake_parse_arguments(PARSE_ARGV 1 _arg "NO_PRECOMPILED_HEADER"
        "CONFIG;WARNING_LEVEL;DIRECTORY" "DISABLE_WARNINGS;DEFINITIONS;INCLUDE_DIRECTORIES;FILES")
    if(_arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "siv3d_source_settings: unexpected arguments ${_arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT DEFINED _arg_CONFIG)
        set(_configs Debug Release)
        set(_cond "1")
    elseif(_arg_CONFIG MATCHES "^(Debug|Release)$")
        set(_configs ${_arg_CONFIG})
        set(_cond "$<CONFIG:${_arg_CONFIG}>")
    else()
        message(FATAL_ERROR "siv3d_source_settings: unsupported CONFIG ${_arg_CONFIG}")
    endif()
    if(_arg_NO_PRECOMPILED_HEADER AND DEFINED _arg_CONFIG)
        message(FATAL_ERROR "siv3d_source_settings: NO_PRECOMPILED_HEADER applies to all configurations")
    endif()

    set(_files "")
    foreach(_path IN LISTS _arg_FILES)
        list(APPEND _files "${CMAKE_SOURCE_DIR}/${_path}")
    endforeach()
    if(_arg_DIRECTORY)
        get_target_property(_sources ${_target} SOURCES)
        foreach(_source IN LISTS _sources)
            file(RELATIVE_PATH _relative "${CMAKE_SOURCE_DIR}" "${_source}")
            cmake_path(IS_PREFIX _arg_DIRECTORY "${_relative}" NORMALIZE _under)
            if(_under)
                list(APPEND _files "${_source}")
            endif()
        endforeach()
    endif()
    if(NOT _files)
        message(FATAL_ERROR "siv3d_source_settings: no files selected")
    endif()

    set(_facts "")
    foreach(_file IN LISTS _files)
        file(RELATIVE_PATH _relative "${CMAKE_SOURCE_DIR}" "${_file}")
        if(DEFINED _arg_WARNING_LEVEL)
            if(NOT _arg_WARNING_LEVEL MATCHES "^[0-4]$")
                message(FATAL_ERROR "siv3d_source_settings: unsupported WARNING_LEVEL ${_arg_WARNING_LEVEL}")
            endif()
            # A later call replaces the level, so it is recorded once final.
            foreach(_config IN LISTS _configs)
                set_property(SOURCE "${_file}" TARGET_DIRECTORY ${_target}
                    PROPERTY SIV3D_WARNING_LEVEL_${_config} ${_arg_WARNING_LEVEL})
            endforeach()
        endif()
        foreach(_number IN LISTS _arg_DISABLE_WARNINGS)
            set_property(SOURCE "${_file}" TARGET_DIRECTORY ${_target}
                APPEND PROPERTY COMPILE_OPTIONS "$<${_cond}:/wd${_number}>")
            foreach(_config IN LISTS _configs)
                list(APPEND _facts "${_relative}|${_config}|nowarn|${_number}")
            endforeach()
        endforeach()
        foreach(_definition IN LISTS _arg_DEFINITIONS)
            set_property(SOURCE "${_file}" TARGET_DIRECTORY ${_target}
                APPEND PROPERTY COMPILE_DEFINITIONS "$<${_cond}:${_definition}>")
            foreach(_config IN LISTS _configs)
                list(APPEND _facts "${_relative}|${_config}|define|${_definition}")
            endforeach()
        endforeach()
        foreach(_dir IN LISTS _arg_INCLUDE_DIRECTORIES)
            set_property(SOURCE "${_file}" TARGET_DIRECTORY ${_target}
                APPEND PROPERTY INCLUDE_DIRECTORIES "$<${_cond}:${CMAKE_SOURCE_DIR}/${_dir}>")
            foreach(_config IN LISTS _configs)
                list(APPEND _facts "${_relative}|${_config}|include|${_dir}")
            endforeach()
        endforeach()
        if(_arg_NO_PRECOMPILED_HEADER)
            set_property(SOURCE "${_file}" TARGET_DIRECTORY ${_target}
                PROPERTY SKIP_PRECOMPILE_HEADERS ON)
            list(APPEND _facts "${_relative}|*|nopch|")
        endif()
    endforeach()
    set_property(TARGET ${_target} APPEND PROPERTY SIV3D_SOURCE_FACTS ${_facts})
endfunction()

function(siv3d_finish_sources _target)
    cmake_parse_arguments(PARSE_ARGV 1 _arg "" "WARNING_LEVEL;FACTS" "")
    if(NOT _arg_WARNING_LEVEL MATCHES "^[0-4]$" OR NOT _arg_FACTS)
        message(FATAL_ERROR "siv3d_finish_sources: WARNING_LEVEL <0-4> and FACTS <file> are required")
    endif()
    get_target_property(_sources ${_target} SOURCES)
    get_target_property(_facts ${_target} SIV3D_SOURCE_FACTS)
    if(NOT _facts)
        set(_facts "")
    endif()

    set(_compiled "")
    set(_overrides FALSE)
    foreach(_source IN LISTS _sources)
        if(NOT _source MATCHES "\\.(c|cc|cpp|rc)$")
            continue()
        endif()
        file(RELATIVE_PATH _relative "${CMAKE_SOURCE_DIR}" "${_source}")
        list(APPEND _facts "${_relative}|*|source|")
        if(_source MATCHES "\\.rc$")
            continue()
        endif()
        list(APPEND _compiled "${_source}")
        foreach(_config Debug Release)
            get_property(_level SOURCE "${_source}" TARGET_DIRECTORY ${_target}
                PROPERTY SIV3D_WARNING_LEVEL_${_config})
            if(NOT "${_level}" STREQUAL "")
                set(_overrides TRUE)
                list(APPEND _facts "${_relative}|${_config}|warning|${_level}")
            endif()
        endforeach()
    endforeach()

    if(_overrides)
        foreach(_source IN LISTS _compiled)
            set(_levels "")
            foreach(_config Debug Release)
                get_property(_level SOURCE "${_source}" TARGET_DIRECTORY ${_target}
                    PROPERTY SIV3D_WARNING_LEVEL_${_config})
                if("${_level}" STREQUAL "")
                    set(_level ${_arg_WARNING_LEVEL})
                endif()
                list(APPEND _levels ${_level})
            endforeach()
            list(GET _levels 0 _debug)
            list(GET _levels 1 _release)
            if(_debug STREQUAL _release)
                set(_option "/W${_debug}")
            else()
                set(_option "$<IF:$<CONFIG:Debug>,/W${_debug},/W${_release}>")
            endif()
            get_property(_options SOURCE "${_source}" TARGET_DIRECTORY ${_target} PROPERTY COMPILE_OPTIONS)
            set_property(SOURCE "${_source}" TARGET_DIRECTORY ${_target}
                PROPERTY COMPILE_OPTIONS ${_option} ${_options})
        endforeach()
        # Checked once the caller has finished configuring the target. A
        # deferred call expands its arguments when it runs, so bind the name now.
        cmake_language(EVAL CODE "cmake_language(DEFER CALL _siv3d_no_precompiled_header [[${_target}]])")
    else()
        target_compile_options(${_target} PRIVATE /W${_arg_WARNING_LEVEL})
    endif()

    list(SORT _facts)
    list(JOIN _facts "\n" _content)
    file(WRITE "${_arg_FACTS}" "${_content}\n")
endfunction()

function(_siv3d_no_precompiled_header _target)
    get_target_property(_pch ${_target} PRECOMPILE_HEADERS)
    if(_pch)
        message(FATAL_ERROR "${_target}: a precompiled header gets no warning level "
            "when warning levels are set per file (C4652)")
    endif()
endfunction()
