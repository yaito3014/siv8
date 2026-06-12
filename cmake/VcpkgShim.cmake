# =============================================================================
# VcpkgShim.cmake
#
# Compatibility shim for consuming a vcpkg install tree WITHOUT the vcpkg
# toolchain file. Installed vcpkg packages rely on two toolchain behaviors:
#
#   1. find_package() dispatches through the port's
#      share/<name>/vcpkg-cmake-wrapper.cmake. E.g. freetype's static config
#      references ZLIB::ZLIB / BZip2::BZip2 / PNG::PNG / brotli in its link
#      interface and counts on its wrapper to find them.
#   2. <triplet>/debug is on the search path, so module-mode finds (FindBZip2,
#      FindPNG, ...) locate the d-suffixed debug libraries; without it every
#      configuration silently links the release (/MT) libraries.
#
# Usage:
#   include(VcpkgShim.cmake)
#   vcpkg_shim_use_prefix(<...>/vcpkg_installed/<triplet>)   # explicit tree
#   vcpkg_shim_autodetect()       # or: scan CMAKE_PREFIX_PATH for one
#
# Everything backs off when the real toolchain is loaded (VCPKG_TOOLCHAIN) or
# when find_package is already overridden (COMMAND _find_package).
# =============================================================================

# Activate one installed tree: put <root> and <root>/debug on CMAKE_PREFIX_PATH
# (debug first for Debug or multi-config builds, mirroring the toolchain's
# z_vcpkg_add_vcpkg_to_cmake_path) and provide the context variables the port
# wrappers and configs read. Must be a macro: the effects belong to the
# caller's directory scope.
macro(vcpkg_shim_use_prefix z_vcpkg_shim_root)
    if(NOT VCPKG_TOOLCHAIN)
        get_filename_component(z_vcpkg_shim_abs "${z_vcpkg_shim_root}" ABSOLUTE)
        if(NOT DEFINED _VCPKG_INSTALLED_DIR)
            get_filename_component(_VCPKG_INSTALLED_DIR "${z_vcpkg_shim_abs}" DIRECTORY)
        endif()
        if(NOT DEFINED VCPKG_TARGET_TRIPLET)
            get_filename_component(VCPKG_TARGET_TRIPLET "${z_vcpkg_shim_abs}" NAME)
        endif()
        set(z_vcpkg_shim_paths "${z_vcpkg_shim_abs}" "${z_vcpkg_shim_abs}/debug")
        if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
            list(REVERSE z_vcpkg_shim_paths) # Debug/multi-config: debug paths first
        endif()
        list(INSERT CMAKE_PREFIX_PATH 0 ${z_vcpkg_shim_paths})
        list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH) # keeps the first (our) occurrence
        unset(z_vcpkg_shim_paths)
        unset(z_vcpkg_shim_abs)
    endif()
endmacro()

# Activate the first CMAKE_PREFIX_PATH entry that looks like a vcpkg installed
# tree: <root>/share plus the vcpkg/ status directory next to <root>.
macro(vcpkg_shim_autodetect)
    if(NOT VCPKG_TOOLCHAIN AND NOT DEFINED _VCPKG_INSTALLED_DIR)
        foreach(z_vcpkg_shim_candidate IN LISTS CMAKE_PREFIX_PATH)
            get_filename_component(z_vcpkg_shim_candidate_abs "${z_vcpkg_shim_candidate}" ABSOLUTE)
            get_filename_component(z_vcpkg_shim_candidate_parent "${z_vcpkg_shim_candidate_abs}" DIRECTORY)
            if(IS_DIRECTORY "${z_vcpkg_shim_candidate_abs}/share"
                    AND IS_DIRECTORY "${z_vcpkg_shim_candidate_parent}/vcpkg")
                vcpkg_shim_use_prefix("${z_vcpkg_shim_candidate_abs}")
                break()
            endif()
        endforeach()
        unset(z_vcpkg_shim_candidate_abs)
        unset(z_vcpkg_shim_candidate_parent)
    endif()
endmacro()

# find_package dispatch through the port wrappers, modeled on the toolchain's
# override (including the depth-indexed ARGS backup: wrappers call
# find_package recursively). Falls through to the builtin when no tree is
# active or the port has no wrapper.
if(NOT VCPKG_TOOLCHAIN AND NOT COMMAND _find_package)
    set(z_vcpkg_shim_fp_depth 0)
    macro(find_package z_vcpkg_shim_fp_name)
        math(EXPR z_vcpkg_shim_fp_depth "${z_vcpkg_shim_fp_depth} + 1")
        set(z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_wrapper "")
        if(DEFINED _VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
            string(TOLOWER "${z_vcpkg_shim_fp_name}" z_vcpkg_shim_fp_lower)
            set(z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_wrapper
                "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/${z_vcpkg_shim_fp_lower}/vcpkg-cmake-wrapper.cmake")
            unset(z_vcpkg_shim_fp_lower)
        endif()
        if(EXISTS "${z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_wrapper}")
            if(DEFINED ARGS)
                set(z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_args_backup "${ARGS}")
            endif()
            set(ARGS "${z_vcpkg_shim_fp_name};${ARGN}")
            include("${z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_wrapper}")
            if(DEFINED z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_args_backup)
                set(ARGS "${z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_args_backup}")
                unset(z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_args_backup)
            else()
                unset(ARGS)
            endif()
        else()
            _find_package("${z_vcpkg_shim_fp_name}" ${ARGN})
        endif()
        unset(z_vcpkg_shim_fp_${z_vcpkg_shim_fp_depth}_wrapper)
        math(EXPR z_vcpkg_shim_fp_depth "${z_vcpkg_shim_fp_depth} - 1")
    endmacro()
endif()
