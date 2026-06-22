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

# C is required: the vendored ThirdParty tree (plutovg, miniaudio, ...) is C.
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
if(NOT WIN32)
    find_package(glfw3 CONFIG REQUIRED)
endif()

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

# GLFW comes from the overlay fork (overlay-ports/glfw3), not vendored.
if(WIN32)
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/oscpack/ip/posix/")
else()
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/oscpack/ip/win32/")
endif()

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
        "Siv3D/Clipboard/ClipboardFactory\\.cpp"
        "Siv3D/Cursor/CursorFactory\\.cpp"
        "Siv3D/CursorStyle/CursorStyleFactory\\.cpp"
        "Siv3D/DirectoryWatcher/SivDirectoryWatcher\\.cpp"
        "Siv3D/DragDrop/DragDropFactory\\.cpp"
        "Siv3D/Keyboard/KeyboardFactory\\.cpp"
        "Siv3D/MediaTranscoder/MediaTranscoderFactory\\.cpp"
        "Siv3D/Mouse/MouseFactory\\.cpp"
        "Siv3D/NativeShare/NativeShareFactory\\.cpp"
        "Siv3D/Notifications/NotificationsFactory\\.cpp"
        "Siv3D/Notifications/SivNotifications\\.cpp"
        "Siv3D/Pentablet/PentabletFactory\\.cpp"
        "Siv3D/System/SystemFactory\\.cpp"
        "Siv3D/TextToSpeech/TextToSpeechFactory\\.cpp"
        "Siv3D/Window/WindowFactory\\.cpp"
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
    # TODO(linux): no Window/Renderer backend yet — builds common code only.
    file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux/*.cpp"
    )
    # The GLFW-based input layer lives under macOS/ only for now.
    list(FILTER SIV3D_PLATFORM_SOURCES EXCLUDE REGEX "/macOS_Linux/Siv3D/(CursorStyle|Keyboard|Mouse)/")
    message(WARNING "Siv3D: Linux has no graphics backend yet — common code only. See TODO(linux).")
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

# PUBLIC via genex so the flags stay in the exported interface.
target_compile_options(Siv3DCore PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8;/Zc:__cplusplus;/MP>")
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
    target_include_directories(Siv3DCore PRIVATE "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux")
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

# PUBLIC: these appear in public Siv3D headers, so consumers need their includes
# (and the shared include root also covers config-less pcg / parallel-hashmap).
target_link_libraries(Siv3DCore PUBLIC
    Microsoft::DirectXMath
    nlohmann_json::nlohmann_json
    nanoflann::nanoflann
    fmt::fmt
    toml11::toml11
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
    find_library(COCOA_LIB    Cocoa     REQUIRED)
    find_library(METAL_LIB    Metal     REQUIRED)
    find_library(METALKIT_LIB MetalKit  REQUIRED)
    find_library(QUARTZ_LIB   QuartzCore REQUIRED)
    find_library(IOKIT_LIB    IOKit     REQUIRED)
    target_link_libraries(Siv3DCore PRIVATE
        ${COCOA_LIB} ${METAL_LIB} ${METALKIT_LIB} ${QUARTZ_LIB} ${IOKIT_LIB}
    )
elseif(UNIX)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(Siv3DCore PRIVATE glfw)
endif()

# zstd is built with ZSTD_MULTITHREAD -> needs pthread on POSIX.
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(Siv3DCore PUBLIC Threads::Threads)
endif()

# ============================================================================
# Fat library: libSiv3D.a — engine + every dependency, content-deduped.
# Default ON for a top-level build, OFF under add_subdirectory.
# ============================================================================
if(PROJECT_IS_TOP_LEVEL)
    set(_siv3d_fat_default ON)
else()
    set(_siv3d_fat_default OFF)
endif()
option(SIV3D_FAT_LIB "Build the fat libSiv3D.a bundling all deps" ${_siv3d_fat_default})
if(SIV3D_FAT_LIB)
    set(_siv3d_fat "${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Siv3D${CMAKE_STATIC_LIBRARY_SUFFIX}")
    if(MSVC)
        # lib.exe merges without content-dedup (harmless under normal linking).
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND lib.exe /NOLOGO "/OUT:${_siv3d_fat}" "$<TARGET_FILE:Siv3DCore>"
                    "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/*.lib"
            DEPENDS Siv3DCore VERBATIM)
    else()
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND "${CMAKE_COMMAND}"
                "-DSIV3D_MAIN_LIB=$<TARGET_FILE:Siv3DCore>"
                "-DSIV3D_DEP_LIBDIR=${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib"
                "-DSIV3D_FATLIB_OUT=${_siv3d_fat}"
                "-DSIV3D_AR=${CMAKE_AR}" "-DSIV3D_RANLIB=${CMAKE_RANLIB}"
                "-DSIV3D_WORK=${CMAKE_BINARY_DIR}/_fatlib"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake"
            DEPENDS Siv3DCore "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake"
            COMMENT "Building deduped fat static lib: ${_siv3d_fat}" VERBATIM)
    endif()
    add_custom_target(Siv3D-fat ALL DEPENDS "${_siv3d_fat}")
endif()

# ============================================================================
# Install: fat libSiv3D.a + headers (usable from any build system, or CMake).
# ============================================================================
if(NOT SIV3D_FAT_LIB)
    message(WARNING "Siv3D: SIV3D_FAT_LIB is OFF — the default install has no library.")
else()
    install(FILES "${_siv3d_fat}" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

# Public headers, nested under include/Siv3D to avoid polluting a shared prefix.
install(DIRECTORY "${SIV3D_INCLUDE}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")

# vcpkg headers the PUBLIC Siv3D headers include (private deps live in the fat lib).
set(_siv3d_depinc "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
install(DIRECTORY
        "${_siv3d_depinc}/nlohmann"
        "${_siv3d_depinc}/absl"
        "${_siv3d_depinc}/parallel_hashmap"
        "${_siv3d_depinc}/fmt"
        "${_siv3d_depinc}/toml11"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")
install(FILES
        "${_siv3d_depinc}/mimalloc.h"
        "${_siv3d_depinc}/toml.hpp"
        "${_siv3d_depinc}/nanoflann.hpp"
        "${_siv3d_depinc}/pcg_random.hpp"
        "${_siv3d_depinc}/pcg_extras.hpp"
        "${_siv3d_depinc}/pcg_uint128.hpp"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")
# DirectXMath.h includes <sal.h>; ship the directxmath SAL shim where present.
file(GLOB _siv3d_dxmath "${_siv3d_depinc}/DirectX*.h" "${_siv3d_depinc}/DirectX*.inl")
if(EXISTS "${_siv3d_depinc}/sal.h")
    list(APPEND _siv3d_dxmath "${_siv3d_depinc}/sal.h")
endif()
install(FILES ${_siv3d_dxmath} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")

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
# Single-option genexes so they survive as a list. Linux X11/GL libs TBD.
set(SIV3D_FAT_COMPILE_OPTIONS
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>;$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    string(APPEND SIV3D_FAT_COMPILE_OPTIONS ";$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.2>")
endif()
if(WIN32)
    set(SIV3D_FAT_SYSTEM_LIBS "d3d11;dxgi;dxguid;d3dcompiler;winmm;ws2_32;imm32;version")
elseif(APPLE)
    set(SIV3D_FAT_SYSTEM_LIBS "-framework Cocoa;-framework Metal;-framework MetalKit;-framework QuartzCore;-framework IOKit")
else()
    set(SIV3D_FAT_SYSTEM_LIBS "Threads::Threads;${CMAKE_DL_LIBS};m")
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
    if(SIV3D_TEST_SOURCES)
        add_executable(Siv3D-Test ${SIV3D_TEST_SOURCES})
        target_link_libraries(Siv3D-Test PRIVATE Siv3D::Siv3DCore doctest::doctest nanobench::nanobench)
    endif()
endif()
