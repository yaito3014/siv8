# CMake build of WindowsDesktop/OpenSiv3D.slnx: the Siv3D static library and
# the Siv3D-Test application, with the sources and settings of the Visual
# Studio projects, the vendored third-party code under Siv3D/src/ThirdParty,
# and the prebuilt libraries in Siv3D/lib/Windows. ctest checks the sources
# and per-file settings against the projects. Windows (x64, MSVC) only; macOS
# keeps its Xcode project.
cmake_minimum_required(VERSION 3.25)

# The prebuilt libraries exist for these two configurations only.
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "Build configurations")

project(Siv3D LANGUAGES C CXX)

if(NOT MSVC OR NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
    message(FATAL_ERROR "The CMake build of Siv3D supports Windows x64 with MSVC only; "
        "on macOS, use macOS/OpenSiv3D.xcodeproj.")
endif()
get_property(_siv3d_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_siv3d_multi_config)
    set(_siv3d_configs ${CMAKE_CONFIGURATION_TYPES})
else()
    set(_siv3d_configs "${CMAKE_BUILD_TYPE}")
endif()
foreach(_siv3d_config IN LISTS _siv3d_configs)
    if(NOT _siv3d_config MATCHES "^(Debug|Release)$")
        message(FATAL_ERROR "Unsupported configuration '${_siv3d_config}': the prebuilt "
            "libraries in Siv3D/lib/Windows exist for Debug and Release only "
            "(set CMAKE_BUILD_TYPE, or CMAKE_CONFIGURATION_TYPES, accordingly).")
    endif()
endforeach()

set(SIV3D_BOOST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/boost_1_88_0")
if(NOT EXISTS "${SIV3D_BOOST_DIR}/boost/version.hpp")
    message(FATAL_ERROR "The Boost 1.88.0 headers are missing: unpack the boost/ "
        "directory of boost_1_88_0 into ${SIV3D_BOOST_DIR}.")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DSourceSettings.cmake")
foreach(_siv3d_list
        SIV3D_ENGINE_SOURCES SIV3D_TEST_SOURCES SIV3D_PUBLIC_HEADERS
        SIV3D_PUBLIC_THIRDPARTY_HEADERS SIV3D_SRC_HEADERS SIV3D_PLATFORM_HEADERS
        SIV3D_THIRDPARTY_HEADERS SIV3D_SKIA_HEADERS SIV3D_PREBUILT_HEADERS
        SIV3D_FREETYPE_HEADERS SIV3D_TEST_HEADERS)
    list(TRANSFORM ${_siv3d_list} PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
endforeach()

# The two projects build Debug differently (the library optimized and without
# run-time checks, the application with Visual Studio's defaults), so the
# optimization flags are set per target instead of through CMake's Debug
# defaults, whose /RTC1 cannot be combined with /O2.
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_DEBUG "")

set(_siv3d_windows "${CMAKE_CURRENT_SOURCE_DIR}/WindowsDesktop")
set(_siv3d_root "${CMAKE_CURRENT_SOURCE_DIR}/Siv3D")

# Settings both projects share. The C runtime is the static one (/MT, /MTd),
# which the prebuilt libraries are built with.
function(_siv3d_common_settings _target)
    set_target_properties(${_target} PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(${_target} PRIVATE
        $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
        UNICODE _UNICODE)
    target_compile_options(${_target} PRIVATE
        /sdl /permissive- /Zc:__cplusplus /utf-8
        $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>
        $<$<CONFIG:Release>:/Gy /Oi>)
    if(CMAKE_GENERATOR MATCHES "^Visual Studio")
        target_compile_options(${_target} PRIVATE /MP)
    endif()
endfunction()

# ----------------------------------------------------------------------------
# Siv3D: WindowsDesktop/Siv3D.vcxproj
# ----------------------------------------------------------------------------
add_library(Siv3D STATIC ${SIV3D_ENGINE_SOURCES})
# The include directories are the file sets' base directories, in the order of
# the project's include path.
target_sources(Siv3D
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS "${_siv3d_root}/include"
            FILES ${SIV3D_PUBLIC_HEADERS}
        FILE_SET public_thirdparty TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/include/ThirdParty"
            FILES ${SIV3D_PUBLIC_THIRDPARTY_HEADERS}
    PRIVATE
        FILE_SET src TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src"
            FILES ${SIV3D_SRC_HEADERS}
        FILE_SET platform TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src/Siv3D-Platform/WindowsDesktop"
            FILES ${SIV3D_PLATFORM_HEADERS}
        FILE_SET thirdparty TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src/ThirdParty"
            FILES ${SIV3D_THIRDPARTY_HEADERS}
        FILE_SET skia TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src/ThirdParty/skia"
            FILES ${SIV3D_SKIA_HEADERS}
        FILE_SET prebuilt TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src/ThirdParty-prebuilt"
            FILES ${SIV3D_PREBUILT_HEADERS}
        FILE_SET freetype TYPE HEADERS
            BASE_DIRS "${_siv3d_root}/src/ThirdParty-prebuilt/freetype"
            FILES ${SIV3D_FREETYPE_HEADERS})
# The Boost headers are not part of the repository's lists: each developer
# unpacks them, as for the Visual Studio build.
target_include_directories(Siv3D PRIVATE "${SIV3D_BOOST_DIR}")
_siv3d_common_settings(Siv3D)
siv3d_source_settings(Siv3D WARNING_LEVEL 1 DIRECTORY Siv3D/src/ThirdParty/skia)
siv3d_source_settings(Siv3D DEFINITIONS SK_DISABLE_LEGACY_PNG_WRITEBUFFER
    FILES Siv3D/src/ThirdParty/skia/src/core/SkWriteBuffer.cpp)
siv3d_source_settings(Siv3D WARNING_LEVEL 3 DIRECTORY Siv3D/src/ThirdParty/Oniguruma)
siv3d_source_settings(Siv3D WARNING_LEVEL 0 FILES Siv3D/src/ThirdParty/Oniguruma/st.c)
siv3d_source_settings(Siv3D DISABLE_WARNINGS 4559
    FILES Siv3D/src/Siv3D-Platform/WindowsDesktop/Siv3D/Memory/SivMemory.cpp)
siv3d_source_settings(Siv3D DISABLE_WARNINGS 4100 4132
    FILES Siv3D/src/ThirdParty/cpu_features/impl_x86_windows.c)
siv3d_source_settings(Siv3D DISABLE_WARNINGS 4127 4310 4505
    FILES Siv3D/src/ThirdParty/simdutf/simdutf.cpp)
siv3d_source_settings(Siv3D CONFIG Debug DISABLE_WARNINGS 4201 4456
    FILES Siv3D/src/ThirdParty/MikkTSpace/mikktspace.c)
siv3d_finish_sources(Siv3D WARNING_LEVEL 4 FACTS "${CMAKE_BINARY_DIR}/Siv3D-sources.txt")
# Siv3D.hpp links the library by name (Siv3D/Windows/Libraries.hpp), as
# siv3d/Siv3D or siv3d/Siv3D_d in a library directory: build/lib here.
set_target_properties(Siv3D PROPERTIES
    OUTPUT_NAME_DEBUG Siv3D_d
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/siv3d$<0:>"
    MSVC_DEBUG_INFORMATION_FORMAT "")
target_compile_definitions(Siv3D PRIVATE
    _LIB SIV3D_LIBRARY_BUILD _CRT_SECURE_NO_WARNINGS
    ZSTD_MULTITHREAD ONIG_STATIC MUPARSER_STATIC
    SK_RELEASE SK_ENABLE_OPTIMIZE_SIZE SK_DISABLE_TRACING SK_DISABLE_EFFECT_DESERIALIZATION
    LUNASVG_BUILD_STATIC OSC_HOST_LITTLE_ENDIAN R8B_PFFFT_DOUBLE TOML11_COMPILE_SOURCES)
target_compile_options(Siv3D PRIVATE /O2)
# The prebuilt libraries Siv3D.hpp links by name, and the library itself.
target_link_directories(Siv3D INTERFACE
    "${CMAKE_BINARY_DIR}/lib"
    "${_siv3d_root}/lib/Windows")

# ----------------------------------------------------------------------------
# Siv3D-Test: WindowsDesktop/Siv3D-Test.vcxproj
# ----------------------------------------------------------------------------
add_executable(Siv3D-Test WIN32 ${SIV3D_TEST_SOURCES} "${_siv3d_windows}/codepage.manifest")
target_sources(Siv3D-Test PRIVATE
    FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/Test" "${_siv3d_windows}"
        FILES ${SIV3D_TEST_HEADERS})
_siv3d_common_settings(Siv3D-Test)
siv3d_source_settings(Siv3D-Test
    INCLUDE_DIRECTORIES Siv3D/src Siv3D/src/Siv3D-Platform/WindowsDesktop
    FILES
        Test/Test_ConstantBuffer.cpp
        Test/Test_D3D11Device.cpp
        Test/Test_D3D11DisplayFrequency.cpp
        Test/Test_D3D11SwapChain.cpp
        Test/Test_MonitorInfo.cpp)
siv3d_source_settings(Siv3D-Test NO_PRECOMPILED_HEADER DEFINITIONS CATCH_AMALGAMATED_CUSTOM_MAIN
    FILES Test/ThirdParty/Catch2/catch_amalgamated.cpp)
siv3d_finish_sources(Siv3D-Test WARNING_LEVEL 4 FACTS "${CMAKE_BINARY_DIR}/Siv3D-Test-sources.txt")
target_link_libraries(Siv3D-Test PRIVATE Siv3D)
target_compile_definitions(Siv3D-Test PRIVATE _WINDOWS)
target_compile_options(Siv3D-Test PRIVATE
    /wd26498 /JMC-
    $<IF:$<CONFIG:Debug>,/Od /RTC1,/O2>)
target_precompile_headers(Siv3D-Test PRIVATE "${_siv3d_windows}/pch.h")
target_link_options(Siv3D-Test PRIVATE
    /DEBUG
    $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /DELAYLOAD:winmm.dll>)
target_link_libraries(Siv3D-Test PRIVATE $<$<CONFIG:Release>:delayimp>)
set_target_properties(Siv3D-Test PROPERTIES
    OUTPUT_NAME_DEBUG "Siv3D-Test(debug)"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase
    VS_DEBUGGER_WORKING_DIRECTORY "${_siv3d_windows}/App"
    VS_DEBUGGER_ENVIRONMENT "SIV3D_APP_LAUNCHED_FROM_IDE=1")
# The application runs from WindowsDesktop/App, next to its resources.
add_custom_command(TARGET Siv3D-Test POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:Siv3D-Test>" "${_siv3d_windows}/App/"
    VERBATIM)

# ----------------------------------------------------------------------------
# ctest -C <config>: runs the copy in WindowsDesktop/App. The application has
# no console, so its report is read from a file (cmake/RunSiv3DTest.cmake).
# ----------------------------------------------------------------------------
enable_testing()
add_test(NAME Siv3D-Test
    COMMAND "${CMAKE_COMMAND}"
        "-DEXE=${_siv3d_windows}/App/$<TARGET_FILE_NAME:Siv3D-Test>"
        "-DREPORT=${CMAKE_BINARY_DIR}/Siv3D-Test-$<CONFIG>.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunSiv3DTest.cmake")
# A Debug CRT assertion opens a modal dialog and would hang an unattended run.
set_tests_properties(Siv3D-Test PROPERTIES TIMEOUT 1800)

# The sources and per-file settings above must match the Visual Studio
# projects', which stay authoritative while both builds exist.
foreach(_siv3d_target Siv3D Siv3D-Test)
    add_test(NAME ${_siv3d_target}.vcxproj
        COMMAND "${CMAKE_COMMAND}"
            "-DVCXPROJ=${_siv3d_windows}/${_siv3d_target}.vcxproj"
            "-DFACTS=${CMAKE_BINARY_DIR}/${_siv3d_target}-sources.txt"
            "-DROOT=${CMAKE_CURRENT_SOURCE_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DCheckVcxproj.cmake")
endforeach()
