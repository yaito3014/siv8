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

# C is required: the vendored ThirdParty miniaudio implementation TU is C.
project(Siv3D LANGUAGES C CXX VERSION 0.8.0)

if(APPLE)
    enable_language(OBJCXX)
endif()

include(GNUInstallDirs)

# MSVC: take the C runtime from the vcpkg triplet unless the including project
# chose one, which must then agree with the triplet.
if(MSVC)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DMSVCRuntime.cmake")
    siv3d_select_msvc_runtime()
endif()

set(SIV3D_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/Siv3D")
set(SIV3D_SRC     "${SIV3D_DIR}/src")
set(SIV3D_INCLUDE "${SIV3D_DIR}/include")

# ============================================================================
# Binary dependencies (vcpkg manifest, static)
# ============================================================================
# Each element holds one package's find_package arguments, space-separated.
# The same list generates the find_dependency calls in Siv3DConfig.cmake: the
# exported link interface of a static library keeps its PRIVATE dependencies
# (as $<LINK_ONLY:...>), so consumers need every package found here.
set(SIV3D_DEPENDENCIES
    # Freetype via CONFIG so harfbuzz reuses the same target.
    "Freetype CONFIG"
    "harfbuzz CONFIG"
    "PNG"
    "libjpeg-turbo CONFIG"
    "ZLIB"
    "absl CONFIG"
    "directxmath CONFIG"
    "nlohmann_json CONFIG"
    "nanoflann CONFIG"
    "fmt CONFIG"
    "toml11 CONFIG"
    "OpenCV COMPONENTS core imgproc"
    "mimalloc CONFIG"
    "oniguruma CONFIG"
    "double-conversion CONFIG"
    "muparser CONFIG"
    "simdutf CONFIG"
    "zstd CONFIG"
    "lunasvg CONFIG"
    "CpuFeatures CONFIG"
    "Boost 1.88 CONFIG COMPONENTS filesystem"
    "unofficial-skia CONFIG"
    "msdfgen CONFIG"
    "unofficial-easyexif CONFIG"
    "serial_cpp CONFIG"
    "pffft CONFIG"
    "unofficial-r8brain-free-src CONFIG"
    "unofficial-bc7enc-rdo CONFIG"
    "unofficial-levenshtein-sse CONFIG"
    "unofficial-rectpack2d CONFIG"
    "unofficial-obfuscate CONFIG"
    "unofficial-xoshiro-cpp CONFIG"
    "unofficial-enumbitmask CONFIG"
    "unofficial-libcpprime CONFIG"
)
if(WIN32)
    list(APPEND SIV3D_DEPENDENCIES "unofficial-wintoast CONFIG")
else()
    list(APPEND SIV3D_DEPENDENCIES "glfw3 CONFIG" "Threads")
    if(NOT APPLE)
        # OpenGL 4.1 backend: glad (GL loader) + system GL.
        list(APPEND SIV3D_DEPENDENCIES "glad CONFIG" "OpenGL")
    endif()
endif()

set(_siv3d_find_dependency_calls "")
foreach(_siv3d_dep IN LISTS SIV3D_DEPENDENCIES)
    separate_arguments(_siv3d_dep_args UNIX_COMMAND "${_siv3d_dep}")
    find_package(${_siv3d_dep_args} REQUIRED)
    list(APPEND _siv3d_find_dependency_calls "    find_dependency(${_siv3d_dep})")
endforeach()
list(JOIN _siv3d_find_dependency_calls "\n" SIV3D_FIND_DEPENDENCY_CALLS)

# oscpack (OSC networking) comes from vcpkg (formerly vendored in ThirdParty/).
# The port exports no CMake config, so locate the headers + static lib directly.
find_path(OSCPACK_INCLUDE_DIR "osc/OscOutboundPacketStream.h" REQUIRED)
find_library(OSCPACK_LIBRARY oscpack REQUIRED)

# Fail early on a triplet with dynamic libraries (the static targets are absent).
if(NOT TARGET mimalloc-static OR NOT TARGET zstd::libzstd_static)
    message(FATAL_ERROR
        "Siv3D requires a vcpkg triplet with static libraries (got '${VCPKG_TARGET_TRIPLET}'): "
        "<arch>-windows-static (/MT) or <arch>-windows-static-md (/MD) on Windows, "
        "x64-linux, x64-osx or arm64-osx elsewhere.")
endif()

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

# macOS_Linux/ is shared by the macOS and Linux (GLFW) ports, including the
# GLFW input classes.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DPlatform.cmake")
siv3d_platform_dirs(SIV3D_PLATFORM_DIRS "${SIV3D_SRC}")
set(_siv3d_platform_patterns "")
foreach(_siv3d_dir IN LISTS SIV3D_PLATFORM_DIRS)
    list(APPEND _siv3d_platform_patterns "${_siv3d_dir}/*.cpp")
    if(APPLE)
        list(APPEND _siv3d_platform_patterns "${_siv3d_dir}/*.mm")
    endif()
endforeach()
file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS ${_siv3d_platform_patterns})

# Headers, listed so that IDE generators show them under Siv3DCore.
function(_siv3d_glob_headers out_var)
    set(_patterns "")
    foreach(_dir IN LISTS ARGN)
        list(APPEND _patterns "${_dir}/*.h" "${_dir}/*.H" "${_dir}/*.hpp" "${_dir}/*.ipp")
    endforeach()
    file(GLOB_RECURSE _headers CONFIGURE_DEPENDS ${_patterns})
    # *.h and *.H match the same files on case-insensitive file systems.
    list(REMOVE_DUPLICATES _headers)
    set(${out_var} "${_headers}" PARENT_SCOPE)
endfunction()
_siv3d_glob_headers(SIV3D_PUBLIC_HEADERS "${SIV3D_INCLUDE}")
_siv3d_glob_headers(SIV3D_COMMON_HEADERS "${SIV3D_SRC}/Siv3D")
_siv3d_glob_headers(SIV3D_TP_HEADERS "${SIV3D_SRC}/ThirdParty")
_siv3d_glob_headers(SIV3D_PLATFORM_HEADERS ${SIV3D_PLATFORM_DIRS})

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

# Usage requirements for consumers of the headers. The fat-lib target in
# Siv3DConfig.cmake gets the same lists, so keep one option per genex: each
# element must survive substitution into the config as a list item.
set(SIV3D_PUBLIC_COMPILE_OPTIONS
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
# SSE4.2 baseline (x86-64 only) for the SIMD headers.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    list(APPEND SIV3D_PUBLIC_COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.2>")
endif()
set(SIV3D_PUBLIC_COMPILE_DEFINITIONS "")
if(MSVC)
    # Int128.hpp includes absl/numeric/int128.h, which uses
    # std::float_denorm_style, deprecated in C++23 by the MSVC STL.
    list(APPEND SIV3D_PUBLIC_COMPILE_DEFINITIONS _SILENCE_CXX23_DENORM_DEPRECATION_WARNING)
endif()

# /MP is just build-speed for our own TUs (VS generator), so it stays PRIVATE.
target_compile_options(Siv3DCore
    PUBLIC  ${SIV3D_PUBLIC_COMPILE_OPTIONS}
    PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/MP>")
target_compile_definitions(Siv3DCore PUBLIC ${SIV3D_PUBLIC_COMPILE_DEFINITIONS})
if(APPLE)
    set_target_properties(Siv3DCore PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
endif()

# Each file set's base directories become include roots, in the order of these
# calls. The public set also installs to include/Siv3D (see install(TARGETS)).
target_sources(Siv3DCore
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${SIV3D_INCLUDE}"
        FILES ${SIV3D_PUBLIC_HEADERS}
    PRIVATE
        FILE_SET siv3d_headers TYPE HEADERS
        BASE_DIRS "${SIV3D_SRC}"
        FILES ${SIV3D_COMMON_HEADERS}
    PRIVATE
        FILE_SET thirdparty_headers TYPE HEADERS
        BASE_DIRS "${SIV3D_SRC}/ThirdParty"
        FILES ${SIV3D_TP_HEADERS}
    PRIVATE
        FILE_SET platform_headers TYPE HEADERS
        BASE_DIRS ${SIV3D_PLATFORM_DIRS}
        FILES ${SIV3D_PLATFORM_HEADERS}
)
# Bundled third-party headers install to include/Siv3D-thirdparty (matches
# the fat-lib target + the install(FILES ...) rules below).
target_include_directories(Siv3DCore PUBLIC
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D-thirdparty>")

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
    )
endif()

# PUBLIC: these appear in public Siv3D headers, so consumers need their includes
# (and the shared include root also covers config-less pcg / parallel-hashmap).
target_link_libraries(Siv3DCore
    PUBLIC
        Microsoft::DirectXMath
        nlohmann_json::nlohmann_json
        nanoflann::nanoflann
        fmt::fmt
        toml11::toml11
        absl::int128
        absl::function_ref
        absl::random_distributions
        mimalloc-static
        unofficial::obfuscate::obfuscate
        unofficial::xoshiro-cpp::xoshiro-cpp
        unofficial::enumbitmask::enumbitmask
        unofficial::libcpprime::libcpprime
    PRIVATE
        unofficial::skia::skia
        msdfgen::msdfgen-ext
        Freetype::Freetype
        harfbuzz::harfbuzz
        PNG::PNG
        ZLIB::ZLIB
        ${OpenCV_LIBS}
        absl::random_random
        oniguruma::onig
        Boost::headers
        Boost::filesystem
        CpuFeatures::cpu_features
        double-conversion::double-conversion
        muparser::muparser
        simdutf::simdutf
        lunasvg::lunasvg
        zstd::libzstd_static
        libjpeg-turbo::jpeg-static
        libjpeg-turbo::turbojpeg-static
        unofficial::bc7enc-rdo::bc7enc_rdo
        unofficial::levenshtein-sse::levenshtein-sse
        unofficial::rectpack2d::rectpack2d
        unofficial::easyexif::easyexif
        unofficial::r8brain-free-src::r8brain-free-src
        serial_cpp::serial_cpp
)

# oscpack: no exported target, so wire the located headers/lib by hand. On
# Windows it needs Winsock (ws2_32) and winmm, both in SIV3D_WINDOWS_SYSTEM_LIBS.
target_include_directories(Siv3DCore PRIVATE ${OSCPACK_INCLUDE_DIR})
target_link_libraries(Siv3DCore PRIVATE ${OSCPACK_LIBRARY})

# Platform system libraries.
if(WIN32)
    # Also linked by fat-lib consumers (SIV3D_FAT_SYSTEM_LIBS below).
    set(SIV3D_WINDOWS_SYSTEM_LIBS d3d11 dxgi dxguid d3dcompiler winmm ws2_32 imm32 version)
    target_link_libraries(Siv3DCore PRIVATE
        ${SIV3D_WINDOWS_SYSTEM_LIBS}
        unofficial::wintoast::wintoast
    )
    target_compile_definitions(Siv3DCore PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE)
elseif(APPLE)
    # GLFW is the macOS window/input backend.
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
    target_link_libraries(Siv3DCore PRIVATE glfw)
    # OpenGL backend: glad (GL 4.1 loader) + system GL.
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

# System libraries for the fat target (the raw .a has none of its own usage
# requirements); its compile options and definitions are Siv3DCore's PUBLIC ones.
if(WIN32)
    set(SIV3D_FAT_SYSTEM_LIBS "${SIV3D_WINDOWS_SYSTEM_LIBS}")
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

# Exported so consumers can build with the same C runtime (empty outside MSVC).
set(SIV3D_MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
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
# The public headers install by default (the fat lib needs them too), nested
# under include/Siv3D to avoid polluting a shared prefix.
install(TARGETS Siv3DCore EXPORT Siv3DCoreTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL
    FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")
install(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
install(DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/"
    DESTINATION "${SIV3D_VCPKG_BUNDLE_RELDIR}/${VCPKG_TARGET_TRIPLET}"
    COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL
    # The SDK is a Release artifact; the debug build of every dep (~1.7 GB on
    # Linux) is dead weight for a release consumer. Drop the debug/ subtree AND
    # the matching *-debug.cmake imported-config files — those set
    # IMPORTED_LOCATION_DEBUG to the now-absent debug libs and would make
    # find_package fail with "not all the files it references".
    PATTERN "debug" EXCLUDE
    PATTERN "*-debug.cmake" EXCLUDE)

# Strip implementation-only dep headers from the installed bundle — a consumer
# never #includes them (boost alone is ~126 MB; the public Siv3D API only pulls
# the curated set in include/Siv3D-thirdparty, and these deps are compiled into
# the lib). Done as an explicit install(CODE) rather than install(DIRECTORY ...
# REGEX EXCLUDE): the latter matches native paths, so a mid-path regex silently
# no-ops on Windows. The share/<pkg> CMake configs stay, so find_dependency still
# resolves these deps (lib + config; the consumer just can't include their headers).
install(CODE "
    set(_siv3d_tp \"\${CMAKE_INSTALL_PREFIX}/${SIV3D_VCPKG_BUNDLE_RELDIR}/${VCPKG_TARGET_TRIPLET}\")
    set(_siv3d_inc \"\${_siv3d_tp}/include\")
    foreach(_siv3d_d boost skia opencv4 freetype freetype2 GL simdutf)
        file(REMOVE_RECURSE \"\${_siv3d_inc}/\${_siv3d_d}\")
    endforeach()
    file(REMOVE \"\${_siv3d_inc}/miniaudio.h\")
    # Unused Skia module libs — the engine links skia core only; skottie/svg/sksg
    # are ~440 MB of dead weight in the release lib/ on Windows. Exact basenames
    # so 'svg' doesn't clip lunasvg.
    foreach(_siv3d_l skottie svg sksg)
        file(REMOVE \"\${_siv3d_tp}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}\${_siv3d_l}${CMAKE_STATIC_LIBRARY_SUFFIX}\")
    endforeach()
" COMPONENT ${_siv3d_cmpt})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DVcpkgShim.cmake"
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)

# Build-tree export for -DSiv3D_DIR=<build>.
export(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/Siv3DCoreTargets.cmake")

# ============================================================================
# Tests (vcpkg 'tests' feature)
# ============================================================================
if(SIV3D_BUILD_TESTS)
    find_package(nanobench CONFIG REQUIRED)
    file(GLOB_RECURSE SIV3D_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    # Linux/macOS test vehicle. On Windows, Siv3DCore drags in Siv3DMainHelper
    # (GUI entry expecting a user Main()), so a console test exe can't link —
    # Windows runs the tests via RunTest() inside Siv3D-App instead.
    if(SIV3D_TEST_SOURCES AND NOT WIN32)
        add_executable(Siv3D-Test ${SIV3D_TEST_SOURCES})
        target_link_libraries(Siv3D-Test PRIVATE Siv3D::Siv3DCore nanobench::nanobench)
        target_compile_definitions(Siv3D-Test PRIVATE CATCH_AMALGAMATED_CUSTOM_MAIN)
        # Some tests include engine internals; same private roots as Siv3DCore.
        target_include_directories(Siv3D-Test PRIVATE
            "${SIV3D_SRC}"
            $<TARGET_PROPERTY:Siv3DCore,INCLUDE_DIRECTORIES>)
        if(APPLE)
            include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DMacOSTestSources.cmake")
            siv3d_configure_macos_test_sources(Siv3D-Test "${SIV3D_SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/Test")
        endif()
    endif()
endif()
