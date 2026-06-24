cmake_minimum_required(VERSION 3.25)

# Dependencies come from vcpkg manifest mode (vcpkg.json + overlay-ports/).
# Auto-select the vcpkg toolchain from VCPKG_ROOT when not given.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

# Options gating vcpkg manifest features must be set before project().
option(SIV3D_BUILD_TESTS "Build the Siv3D-Test target" ON)
if(SIV3D_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

# Static deps + static CRT: the SDK is one self-contained static library.
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg triplet (static)")
endif()
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# C is required: the vendored ThirdParty miniaudio implementation TU is C.
project(Siv3D LANGUAGES C CXX VERSION 0.8.0)

if(APPLE)
    enable_language(OBJCXX)
endif()

include(GNUInstallDirs)

# Disable any user-wide `vcpkg integrate install` (its /I shadows our headers).
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

set(SIV3D_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/Siv3D")
set(SIV3D_SRC     "${SIV3D_DIR}/src")
set(SIV3D_INCLUDE "${SIV3D_DIR}/include")

# ============================================================================
# Binary dependencies (vcpkg manifest, static)
# ============================================================================
# Freetype via CONFIG so harfbuzz reuses the same target.
find_package(Freetype CONFIG REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(PNG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(nanoflann CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(mimalloc CONFIG REQUIRED)
find_package(oniguruma CONFIG REQUIRED)
find_package(double-conversion CONFIG REQUIRED)
find_package(muparser CONFIG REQUIRED)
find_package(simdutf CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(lunasvg CONFIG REQUIRED)
find_package(CpuFeatures CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(unofficial-skia CONFIG REQUIRED)
find_package(msdfgen CONFIG REQUIRED)
find_package(unofficial-easyexif CONFIG REQUIRED)
find_package(serial_cpp CONFIG REQUIRED)
find_package(pffft CONFIG REQUIRED)
find_package(unofficial-r8brain-free-src CONFIG REQUIRED)
find_package(unofficial-bc7enc-rdo CONFIG REQUIRED)
find_package(unofficial-levenshtein-sse CONFIG REQUIRED)
find_package(unofficial-rectpack2d CONFIG REQUIRED)
find_package(unofficial-obfuscate CONFIG REQUIRED)
find_package(unofficial-xoshiro-cpp CONFIG REQUIRED)
find_package(unofficial-enumbitmask CONFIG REQUIRED)
find_package(unofficial-libcpprime CONFIG REQUIRED)
if(NOT WIN32)
    find_package(glfw3 CONFIG REQUIRED)
endif()

# oscpack (OSC networking) comes from vcpkg (formerly vendored in ThirdParty/).
# The port exports no CMake config, so locate the headers + static lib directly.
find_path(OSCPACK_INCLUDE_DIR "osc/OscOutboundPacketStream.h" REQUIRED)
find_library(OSCPACK_LIBRARY oscpack REQUIRED)

# Fail early if a dynamic triplet was used (the static targets are absent).
if(NOT TARGET mimalloc-static OR NOT TARGET zstd::libzstd_static)
    message(FATAL_ERROR
        "Siv3D requires a STATIC vcpkg triplet (got '${VCPKG_TARGET_TRIPLET}'). "
        "Use x64-windows-static / x64-linux / x64-osx (or the arm64 equivalents).")
endif()

set(SIV3D_VCPKG_LIBS
    Freetype::Freetype
    harfbuzz::harfbuzz
    PNG::PNG
    ZLIB::ZLIB
    ${OpenCV_LIBS}
    absl::int128
    absl::function_ref
    absl::random_random
    absl::random_distributions
    oniguruma::onig
    Boost::headers
    Boost::filesystem
    CpuFeatures::cpu_features
    double-conversion::double-conversion
    muparser::muparser
    simdutf::simdutf
    lunasvg::lunasvg
    zstd::libzstd_static
    mimalloc-static
    libjpeg-turbo::jpeg-static
    libjpeg-turbo::turbojpeg-static
    unofficial::bc7enc-rdo::bc7enc_rdo
    unofficial::levenshtein-sse::levenshtein-sse
    unofficial::rectpack2d::rectpack2d
    unofficial::easyexif::easyexif
    unofficial::r8brain-free-src::r8brain-free-src
    serial_cpp::serial_cpp
)

# TODO(pdfium): not on vcpkg; stays prebuilt, wire as an IMPORTED target later.

# ============================================================================
# Vendored ThirdParty sources compiled into the engine
# ============================================================================
file(GLOB_RECURSE SIV3D_TP_SOURCES CONFIGURE_DEPENDS
    "${SIV3D_SRC}/ThirdParty/*.c"
    "${SIV3D_SRC}/ThirdParty/*.cpp"
    "${SIV3D_SRC}/ThirdParty/*.cc"
)

# ============================================================================
# Engine sources (common + per-platform)
# ============================================================================
file(GLOB_RECURSE SIV3D_COMMON_SOURCES CONFIGURE_DEPENDS
    "${SIV3D_SRC}/Siv3D/*.cpp"
)

# Common TUs whose per-platform detail header has no Linux implementation yet.
if(UNIX AND NOT APPLE)
    set(SIV3D_LINUX_UNIMPLEMENTED
        "Siv3D/ChildProcess/SivChildProcess\\.cpp"
        "Siv3D/DirectoryWatcher/SivDirectoryWatcher\\.cpp"
    )
    foreach(pattern IN LISTS SIV3D_LINUX_UNIMPLEMENTED)
        list(FILTER SIV3D_COMMON_SOURCES EXCLUDE REGEX "${pattern}")
    endforeach()
endif()

set(SIV3D_PLATFORM_SOURCES "")
if(WIN32)
    file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${SIV3D_SRC}/Siv3D-Platform/WindowsDesktop/*.cpp"
    )
elseif(APPLE)
    file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${SIV3D_SRC}/Siv3D-Platform/macOS/*.cpp"
        "${SIV3D_SRC}/Siv3D-Platform/macOS/*.mm"
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux/*.cpp"
    )
elseif(UNIX)
    # Linux (GLFW + OpenGL 4.1): Linux/ is Linux-only; macOS_Linux/ is shared
    # with the macOS port (incl. the GLFW input classes).
    file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux/*.cpp"
        "${SIV3D_SRC}/Siv3D-Platform/Linux/*.cpp"
    )
endif()

# ============================================================================
# Engine library: Siv3DCore (-> libSiv3DCore.a)
# ============================================================================
add_library(Siv3DCore STATIC
    ${SIV3D_COMMON_SOURCES}
    ${SIV3D_PLATFORM_SOURCES}
    ${SIV3D_TP_SOURCES}
)
add_library(Siv3D::Siv3DCore ALIAS Siv3DCore)

# C++23 PUBLIC so consumers including Siv3D headers inherit it; OBJCXX for .mm.
target_compile_features(Siv3DCore PUBLIC cxx_std_23)

# EXPORT_NO_SYSTEM: keep the public ThirdParty headers ahead of system/vcpkg copies.
set_target_properties(Siv3DCore PROPERTIES CXX_EXTENSIONS OFF DEBUG_POSTFIX d EXPORT_NO_SYSTEM TRUE)

# PUBLIC (via genex) so consumers parse the headers the same way; /MP is just
# build-speed for our own TUs (VS generator), so it stays PRIVATE.
target_compile_options(Siv3DCore
    PUBLIC  "$<$<CXX_COMPILER_ID:MSVC>:/utf-8;/Zc:__cplusplus>"
    PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>")
if(APPLE)
    set_target_properties(Siv3DCore PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
endif()

# include/ThirdParty is PUBLIC: public headers include siblings as "fmt/...".
target_include_directories(Siv3DCore
    PUBLIC
        "$<BUILD_INTERFACE:${SIV3D_INCLUDE}>"
        "$<BUILD_INTERFACE:${SIV3D_INCLUDE}/ThirdParty>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D/ThirdParty>"
    PRIVATE
        "${SIV3D_SRC}"
        "${SIV3D_SRC}/ThirdParty"
)
target_link_libraries(Siv3DCore PRIVATE unofficial::skia::skia msdfgen::msdfgen-ext)
if(WIN32)
    target_include_directories(Siv3DCore PRIVATE "${SIV3D_SRC}/Siv3D-Platform/WindowsDesktop")
elseif(APPLE)
    target_include_directories(Siv3DCore PRIVATE
        "${SIV3D_SRC}/Siv3D-Platform/macOS"
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux"
    )
elseif(UNIX)
    target_include_directories(Siv3DCore PRIVATE
        "${SIV3D_SRC}/Siv3D-Platform/Linux"
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux"
    )
endif()

target_compile_definitions(Siv3DCore PRIVATE
    SIV3D_LIBRARY_BUILD
    ZSTD_MULTITHREAD
    LUNASVG_BUILD_STATIC
    OSC_HOST_LITTLE_ENDIAN
    R8B_PFFFT_DOUBLE
    _UNICODE
)
if(MSVC)
    target_compile_definitions(Siv3DCore PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
    )
endif()

target_link_libraries(Siv3DCore PRIVATE ${SIV3D_VCPKG_LIBS})

# oscpack: no exported target, so wire the located headers/lib by hand. The static
# library pulls in Winsock on Windows, so the consumer must link ws2_32 + winmm.
target_include_directories(Siv3DCore PRIVATE ${OSCPACK_INCLUDE_DIR})
target_link_libraries(Siv3DCore PRIVATE ${OSCPACK_LIBRARY})
if(WIN32)
    target_link_libraries(Siv3DCore PRIVATE ws2_32 winmm)
endif()

# PUBLIC: these appear in public Siv3D headers, so consumers need their includes
# (and the shared include root also covers config-less pcg / parallel-hashmap).
target_link_libraries(Siv3DCore PUBLIC
    Microsoft::DirectXMath
    nlohmann_json::nlohmann_json
    nanoflann::nanoflann
    fmt::fmt
    toml11::toml11
    unofficial::obfuscate::obfuscate
    unofficial::xoshiro-cpp::xoshiro-cpp
    unofficial::enumbitmask::enumbitmask
    unofficial::libcpprime::libcpprime
)

# SSE4.2 baseline (x86-64 only; PUBLIC so consumers of the SIMD headers get it).
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    target_compile_options(Siv3DCore PUBLIC "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.2>")
endif()

# Platform system libraries.
if(WIN32)
    find_package(unofficial-wintoast CONFIG REQUIRED)               # unofficial::wintoast::wintoast
    target_link_libraries(Siv3DCore PRIVATE
        d3d11 dxgi dxguid d3dcompiler
        winmm ws2_32 imm32 version
        unofficial::wintoast::wintoast
    )
    target_compile_definitions(Siv3DCore PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE)
elseif(APPLE)
    # GLFW is the macOS window/input backend (found above for all non-Windows).
    target_link_libraries(Siv3DCore PRIVATE glfw)
    # System frameworks the macOS platform .mm sources import (Cocoa pulls in
    # AppKit/Foundation). UNVERIFIED — wired by inspecting the imports, not built.
    set(SIV3D_MACOS_FRAMEWORKS
        Cocoa Metal MetalKit QuartzCore IOKit CoreFoundation CoreGraphics
        CoreServices UniformTypeIdentifiers DiskArbitration SystemConfiguration
        ApplicationServices UserNotifications AudioToolbox)
    foreach(_fw IN LISTS SIV3D_MACOS_FRAMEWORKS)
        find_library(SIV3D_FW_${_fw} ${_fw} REQUIRED)
        target_link_libraries(Siv3DCore PRIVATE "${SIV3D_FW_${_fw}}")
    endforeach()
elseif(UNIX)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(Siv3DCore PRIVATE glfw)
    # OpenGL backend: glad (GL 4.1 loader) + system GL.
    find_package(glad CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Siv3DCore PRIVATE glad::glad OpenGL::GL)
    # X11 dev libs: needed when GLFW is built with its X11 backend (the default,
    # and still present in a dual X11+Wayland build). The engine itself makes no
    # direct Xlib/native calls -- GLFW carries the actual windowing link interface.
    find_package(X11 REQUIRED)
    target_link_libraries(Siv3DCore PRIVATE ${X11_LIBRARIES})
    # stduuid's system generator (UUIDValue) calls uuid_generate from libuuid,
    # which ships a pkg-config file (uuid.pc) but no CMake package.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)
    target_link_libraries(Siv3DCore PRIVATE PkgConfig::UUID)
endif()

# zstd is built with ZSTD_MULTITHREAD -> needs pthread on POSIX.
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(Siv3DCore PUBLIC Threads::Threads)
endif()

# Walk a target's transitive link closure and collect the static/unknown imported
# library FILES (so the fat lib bundles only what the engine actually links, not
# every archive vcpkg happened to build — e.g. skottie/svg/sksg are never linked).
function(_siv3d_fat_closure _root _outvar)
    set(_seen "")
    set(_libs "")
    set(_stack "${_root}")
    while(_stack)
        list(POP_BACK _stack _raw)
        set(_item "${_raw}")
        if(_item MATCHES "^\\$<LINK_ONLY:(.+)>$")          # absl & friends
            set(_item "${CMAKE_MATCH_1}")
        elseif(_item MATCHES "\\$<.*:([^:<>]+)>$")          # other config genexes
            set(_item "${CMAKE_MATCH_1}")
        endif()
        string(STRIP "${_item}" _item)
        if(_item STREQUAL "" OR _item IN_LIST _seen)
            continue()
        endif()
        list(APPEND _seen "${_item}")
        if(NOT TARGET "${_item}")
            # raw library path (e.g. oscpack via find_library, brotli/expat from
            # a dependency's interface) rather than an imported target.
            if(_item MATCHES "\\.(a|lib)$" AND EXISTS "${_item}")
                list(APPEND _libs "${_item}")
            endif()
            continue()
        endif()
        get_target_property(_type "${_item}" TYPE)
        if(_type STREQUAL "STATIC_LIBRARY" OR _type STREQUAL "UNKNOWN_LIBRARY")
            get_target_property(_imp "${_item}" IMPORTED)
            if(_imp)
                set(_loc "")
                foreach(_c RELEASE RELWITHDEBINFO MINSIZEREL DEBUG)
                    get_target_property(_try "${_item}" IMPORTED_LOCATION_${_c})
                    if(_try)
                        set(_loc "${_try}")
                        break()
                    endif()
                endforeach()
                if(NOT _loc)
                    get_target_property(_try "${_item}" IMPORTED_LOCATION)
                    if(_try)
                        set(_loc "${_try}")
                    endif()
                endif()
                if(_loc AND _loc MATCHES "\\.(a|lib)$")
                    list(APPEND _libs "${_loc}")
                endif()
            endif()
        endif()
        foreach(_prop INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
            get_target_property(_deps "${_item}" ${_prop})
            if(_deps)
                list(APPEND _stack ${_deps})
            endif()
        endforeach()
    endwhile()
    list(REMOVE_DUPLICATES _libs)
    # vcpkg ships debug variants under <triplet>/debug/lib; a config genex can
    # drag them into the closure. Keep the release tree only.
    list(FILTER _libs EXCLUDE REGEX "/debug/lib/")
    set(${_outvar} "${_libs}" PARENT_SCOPE)
endfunction()

# ============================================================================
# Fat library: libSiv3D.a — engine + every dependency it links, content-deduped.
# Default ON for a top-level build, OFF under add_subdirectory.
# ============================================================================
option(SIV3D_FAT_LIB "Build the fat libSiv3D.a bundling all deps" ${PROJECT_IS_TOP_LEVEL})
if(SIV3D_FAT_LIB)
    _siv3d_fat_closure(Siv3DCore _siv3d_fatdeps)
    list(LENGTH _siv3d_fatdeps _siv3d_fatdeps_n)
    message(STATUS "Siv3D fat lib: bundling ${_siv3d_fatdeps_n} dependency archives (engine link closure)")
    set(_siv3d_fat "${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Siv3D${CMAKE_STATIC_LIBRARY_SUFFIX}")
    if(MSVC)
        # lib.exe merges (deduping by symbol, not content) from a response file of
        # the closure archives — quoted, one per line, to dodge cmd length limits.
        set(_siv3d_rsp "")
        foreach(_l IN LISTS _siv3d_fatdeps)
            string(APPEND _siv3d_rsp "\"${_l}\"\n")
        endforeach()
        file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/_fatlib_inputs.rsp" CONTENT "${_siv3d_rsp}")
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND lib.exe /NOLOGO "/OUT:${_siv3d_fat}" "$<TARGET_FILE:Siv3DCore>"
                    "@${CMAKE_BINARY_DIR}/_fatlib_inputs.rsp"
            DEPENDS Siv3DCore "${CMAKE_BINARY_DIR}/_fatlib_inputs.rsp" VERBATIM)
    else()
        string(REPLACE ";" "\n" _siv3d_fatdeps_nl "${_siv3d_fatdeps}")
        file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/_fatlib_inputs.txt" CONTENT "${_siv3d_fatdeps_nl}\n")
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND "${CMAKE_COMMAND}"
                "-DSIV3D_MAIN_LIB=$<TARGET_FILE:Siv3DCore>"
                "-DSIV3D_INPUTS_FILE=${CMAKE_BINARY_DIR}/_fatlib_inputs.txt"
                "-DSIV3D_FATLIB_OUT=${_siv3d_fat}"
                "-DSIV3D_AR=${CMAKE_AR}" "-DSIV3D_RANLIB=${CMAKE_RANLIB}"
                "-DSIV3D_WORK=${CMAKE_BINARY_DIR}/_fatlib"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake"
            DEPENDS Siv3DCore "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake"
                    "${CMAKE_BINARY_DIR}/_fatlib_inputs.txt"
            COMMENT "Building deduped fat static lib: ${_siv3d_fat}" VERBATIM)
    endif()
    add_custom_target(Siv3D-fat ALL DEPENDS "${_siv3d_fat}")
endif()

# ============================================================================
# Install: fat libSiv3D.a + headers (usable from any build system, or CMake).
# ============================================================================
if(SIV3D_FAT_LIB)
    install(FILES "${_siv3d_fat}" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

# Public headers, nested under include/Siv3D to avoid polluting a shared prefix.
install(DIRECTORY "${SIV3D_INCLUDE}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")

# Third-party headers the public Siv3D headers include go to a sibling
# include/Siv3D-thirdparty (the Siv3D- prefix avoids colliding with system copies
# in a shared prefix), added as a 2nd interface include by Siv3DConfig.cmake.in.
set(SIV3D_THIRDPARTY_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D-thirdparty")
set(_siv3d_depinc "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
install(DIRECTORY
        "${_siv3d_depinc}/nlohmann"
        "${_siv3d_depinc}/absl"
        "${_siv3d_depinc}/parallel_hashmap"
        "${_siv3d_depinc}/fmt"
        "${_siv3d_depinc}/toml11"
    DESTINATION "${SIV3D_THIRDPARTY_INCLUDEDIR}")
install(FILES
        "${_siv3d_depinc}/mimalloc.h"
        "${_siv3d_depinc}/toml.hpp"
        "${_siv3d_depinc}/nanoflann.hpp"
        "${_siv3d_depinc}/pcg_random.hpp"
        "${_siv3d_depinc}/pcg_extras.hpp"
        "${_siv3d_depinc}/pcg_uint128.hpp"
        "${_siv3d_depinc}/obfuscate.h"
        "${_siv3d_depinc}/XoshiroCpp.hpp"
        "${_siv3d_depinc}/EnumBitmask.hpp"
    DESTINATION "${SIV3D_THIRDPARTY_INCLUDEDIR}")
install(DIRECTORY "${_siv3d_depinc}/libcpprime" DESTINATION "${SIV3D_THIRDPARTY_INCLUDEDIR}")
# DirectXMath.h includes <sal.h>; ship the directxmath SAL shim where present.
file(GLOB _siv3d_dxmath "${_siv3d_depinc}/DirectX*.h" "${_siv3d_depinc}/DirectX*.inl")
if(EXISTS "${_siv3d_depinc}/sal.h")
    list(APPEND _siv3d_dxmath "${_siv3d_depinc}/sal.h")
endif()
install(FILES ${_siv3d_dxmath} DESTINATION "${SIV3D_THIRDPARTY_INCLUDEDIR}")

# ============================================================================
# CMake package — find_package(Siv3D):
#   Siv3D::Siv3D      fat lib (default install)
#   Siv3D::Siv3DCore  engine + deps from the bundle ('Develop' component)
# ============================================================================
include(CMakePackageConfigHelpers)
set(SIV3D_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Siv3D")
set(SIV3D_VCPKG_BUNDLE_RELDIR "${CMAKE_INSTALL_DATAROOTDIR}/Siv3D/vcpkg")
set(SIV3D_BOOST_MIN_VERSION 1.88)

# Usage requirements for the fat target (the raw .a has none of its own).
# Single-option genexes so they survive as a list.
set(SIV3D_FAT_COMPILE_OPTIONS
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>;$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    string(APPEND SIV3D_FAT_COMPILE_OPTIONS ";$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.2>")
endif()
if(WIN32)
    set(SIV3D_FAT_SYSTEM_LIBS "d3d11;dxgi;dxguid;d3dcompiler;winmm;ws2_32;imm32;version")
elseif(APPLE)
    # Mirror the engine's framework set so find_package consumers link them too.
    set(SIV3D_FAT_SYSTEM_LIBS "")
    foreach(_fw IN LISTS SIV3D_MACOS_FRAMEWORKS)
        list(APPEND SIV3D_FAT_SYSTEM_LIBS "-framework ${_fw}")
    endforeach()
else()
    # Mirror the engine's elseif(UNIX) link block so fat-lib consumers get the
    # system GUI/runtime libs the bundled engine + glad/glfw reference (UUIDValue
    # needs libuuid; the GL4 backend needs GL/X11). Raw -l names so consumers
    # don't have to find_package these.
    set(SIV3D_FAT_SYSTEM_LIBS "Threads::Threads;${CMAKE_DL_LIBS};m;GL;X11;uuid")
endif()

configure_package_config_file("cmake/Siv3DConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake"
    INSTALL_DESTINATION "${SIV3D_INSTALL_CMAKEDIR}")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    COMPATIBILITY SameMinorVersion)

# Config ships in the default install (-> Siv3D::Siv3D).
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}")

# 'Develop' component (EXCLUDE_FROM_ALL): core archive + bundle + shim.
#   cmake --install <build> --component Develop
set(_siv3d_cmpt Develop)
install(TARGETS Siv3DCore EXPORT Siv3DCoreTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
install(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
install(DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/"
    DESTINATION "${SIV3D_VCPKG_BUNDLE_RELDIR}/${VCPKG_TARGET_TRIPLET}"
    COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DVcpkgShim.cmake"
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)

# Build-tree export for -DSiv3D_DIR=<build>.
export(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/Siv3DCoreTargets.cmake")

# ============================================================================
# Tests (vcpkg 'tests' feature)
# ============================================================================
if(SIV3D_BUILD_TESTS)
    find_package(doctest CONFIG REQUIRED)
    find_package(nanobench CONFIG REQUIRED)
    file(GLOB_RECURSE SIV3D_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    # Linux/macOS test vehicle. On Windows, Siv3DCore drags in Siv3DMainHelper
    # (GUI entry expecting a user Main()), so a console test exe can't link —
    # Windows runs the tests via RunTest() inside Siv3D-App instead.
    if(SIV3D_TEST_SOURCES AND NOT WIN32)
        add_executable(Siv3D-Test ${SIV3D_TEST_SOURCES})
        target_link_libraries(Siv3D-Test PRIVATE Siv3D::Siv3DCore doctest::doctest nanobench::nanobench)
    endif()
endif()
