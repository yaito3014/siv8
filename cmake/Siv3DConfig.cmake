# Entry point of the Siv3D package. With MSVC, every library a program links
# must use the same kind of C runtime (LNK2038), so an install can hold one
# build per kind: mt/ for the static CRT and md/ for the DLL CRT. This file
# picks the build matching the consumer and loads it. Elsewhere the single
# build sits next to this file.
#
# The consumer's kind is that of CMAKE_MSVC_RUNTIME_LIBRARY, where unset means
# CMake's default, the DLL CRT. A project that sets MSVC_RUNTIME_LIBRARY per
# target instead chooses by setting SIV3D_MSVC_CRT to "static" or "DLL" before
# find_package(Siv3D).
if(MSVC)
    if(DEFINED SIV3D_MSVC_CRT)
        set(_siv3d_crt "${SIV3D_MSVC_CRT}")
    elseif(CMAKE_MSVC_RUNTIME_LIBRARY AND NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
        set(_siv3d_crt "static")
    else()
        set(_siv3d_crt "DLL")
    endif()
    if(_siv3d_crt STREQUAL "static")
        set(_siv3d_variant_dir "${CMAKE_CURRENT_LIST_DIR}/mt")
    elseif(_siv3d_crt STREQUAL "DLL")
        set(_siv3d_variant_dir "${CMAKE_CURRENT_LIST_DIR}/md")
    else()
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "SIV3D_MSVC_CRT must be \"static\" or \"DLL\", not \"${SIV3D_MSVC_CRT}\".")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        unset(_siv3d_crt)
        return()
    endif()
else()
    set(_siv3d_variant_dir "${CMAKE_CURRENT_LIST_DIR}")
endif()

if(NOT EXISTS "${_siv3d_variant_dir}/Siv3DVariantConfig.cmake")
    if(MSVC)
        file(GLOB _siv3d_variants RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
            "${CMAKE_CURRENT_LIST_DIR}/*/Siv3DVariantConfig.cmake")
        list(TRANSFORM _siv3d_variants REPLACE "/Siv3DVariantConfig.cmake$" "")
        string(REPLACE ";" ", " _siv3d_variants "${_siv3d_variants}")
        string(CONCAT ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "This Siv3D package has no build for the ${_siv3d_crt} C runtime "
            "(it has: ${_siv3d_variants}). Set CMAKE_MSVC_RUNTIME_LIBRARY or "
            "SIV3D_MSVC_CRT to match one, or install the missing build.")
    else()
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "${_siv3d_variant_dir}/Siv3DVariantConfig.cmake is missing.")
    endif()
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    unset(_siv3d_crt)
    unset(_siv3d_variants)
    unset(_siv3d_variant_dir)
    return()
endif()

include("${_siv3d_variant_dir}/Siv3DVariantConfig.cmake")
unset(_siv3d_crt)
unset(_siv3d_variant_dir)
