cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

# Static dependencies + static CRT (/MT, /MTd), matching the MSBuild projects:
# the engine and apps built against it are self-contained, with no vcpkg DLLs
# to ship. Consumers must use the same triplet and runtime library.
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg triplet (static, no runtime DLLs)")
endif()
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# C is required: the vendored ThirdParty tree (zstd, plutovg, Oniguruma,
# miniaudio, cpu_features, ...) is C — without it the VS generator silently
# drops every .c source from the build.
project(Siv3D LANGUAGES C CXX VERSION 0.8.0)

if(APPLE)
    enable_language(OBJCXX)
endif()

# The C++ standard is a target usage requirement, set per-target below via
# target_compile_features (not the global CMAKE_CXX_STANDARD), so it stays scoped
# to Siv3D and propagates to consumers through PUBLIC.

include(GNUInstallDirs)

# ---- options --------------------------------------------------------------
option(SIV3D_BUILD_TESTS "Build the Siv3D-Test target (needs vcpkg feature 'tests')" OFF)
# ON by default: the font/SVG subsystems reference Skia unconditionally, so an
# engine built without it cannot link an application (only turn this OFF for
# compile-only iteration on the engine itself).
option(SIV3D_USE_SKIA    "Compile the bundled Skia sources (heavy, required to link apps)" ON)

set(SIV3D_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/Siv3D")
set(SIV3D_SRC     "${SIV3D_DIR}/src")
set(SIV3D_INCLUDE "${SIV3D_DIR}/include")

# ============================================================================
# GROUP A — binary libraries supplied by vcpkg (replace Siv3D/lib + ThirdParty-prebuilt)
#   Canonical includes already used by the engine, so these are safe to link now.
# ============================================================================
find_package(Freetype       REQUIRED)               # Freetype::Freetype
find_package(harfbuzz CONFIG REQUIRED)               # harfbuzz::harfbuzz
find_package(PNG            REQUIRED)                 # PNG::PNG
find_package(JPEG          REQUIRED)                 # JPEG::JPEG (libjpeg-turbo)
find_package(libjpeg-turbo CONFIG REQUIRED)          # TurboJPEG API (tjCompress etc.)
find_package(ZLIB          REQUIRED)                 # ZLIB::ZLIB
find_package(OpenCV        REQUIRED COMPONENTS core imgproc)
find_package(mimalloc CONFIG REQUIRED)               # mimalloc / mimalloc-static

set(SIV3D_VCPKG_LIBS
    Freetype::Freetype
    harfbuzz::harfbuzz
    PNG::PNG
    JPEG::JPEG
    ZLIB::ZLIB
    ${OpenCV_LIBS}
)

# NOTE(zstd): the engine includes <ThirdParty/zstd/zstd.h> and compiles the vendored
#   zstd sources via GROUP B, so vcpkg zstd must NOT be linked (duplicate symbols).
#   Remove the vendored copy first if switching to the vcpkg package.

# mimalloc / turbojpeg target names vary (static vs shared).
if(TARGET mimalloc-static)
    list(APPEND SIV3D_VCPKG_LIBS mimalloc-static)
elseif(TARGET mimalloc)
    list(APPEND SIV3D_VCPKG_LIBS mimalloc)
endif()
if(TARGET libjpeg-turbo::turbojpeg-static)
    list(APPEND SIV3D_VCPKG_LIBS libjpeg-turbo::turbojpeg-static)
elseif(TARGET libjpeg-turbo::turbojpeg)
    list(APPEND SIV3D_VCPKG_LIBS libjpeg-turbo::turbojpeg)
endif()

# TODO(pdfium): PDFium is NOT on vcpkg (ConanCenter only). It stays prebuilt for now.
#   Wire it as an IMPORTED target from Siv3D/lib + WindowsDesktop/App / macOS/App when
#   the PDF subsystem is enabled.

# ============================================================================
# GROUP B — source-vendored ThirdParty, compiled into the engine (Phase 1)
#   The engine includes these via the vendored <ThirdParty/...> layout, so the
#   base include dir is Siv3D/src (so ThirdParty/foo/bar.h resolves).
# ============================================================================
file(GLOB_RECURSE SIV3D_TP_SOURCES CONFIGURE_DEPENDS
    "${SIV3D_SRC}/ThirdParty/*.c"
    "${SIV3D_SRC}/ThirdParty/*.cpp"
    "${SIV3D_SRC}/ThirdParty/*.cc"
)

# Skia comes from an explicit list (cmake/Siv3DSkiaSources.cmake), not the glob:
# the vendored tree is pruned, so globbing sweeps in sources whose headers are
# absent (effects, encode, posix ports, ...).
list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/skia/")
if(SIV3D_USE_SKIA)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DSkiaSources.cmake")
    list(APPEND SIV3D_TP_SOURCES ${SIV3D_SKIA_SOURCES})
endif()

# Sources excluded by the vcxproj/Xcode projects: Oniguruma's unicode_*_data.c
# are #included by other TUs (they do not compile standalone) and the engine
# uses only the ASCII/UTF-32 encodings; msdfgen's export/save modules need
# image libraries the engine does not provide to it.
list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/Oniguruma/(big5|cp1251|euc_(jp|jp_prop|kr|tw)|gb18030|iso8859_[0-9]+|koi8(_r)?|sjis(_prop)?|unicode_(egcb|fold|property|wb)_data|utf16_(be|le)|utf32_be|utf8)\\.c$")
list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/msdfgen/core/(export-svg|save-(bmp|fl32|rgba|tiff))\\.cpp$")

# GLFW is only needed on non-Windows (vcpkg also offers it).
if(WIN32)
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/GLFW/")
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/oscpack/ip/posix/")
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/serial_cpp/impl/unix\\.cc$")
else()
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/WinToast/")
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/oscpack/ip/win32/")
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/serial_cpp/impl/win\\.cc$")
endif()

# ============================================================================
# Engine sources (common + per-platform)
# ============================================================================
file(GLOB_RECURSE SIV3D_COMMON_SOURCES CONFIGURE_DEPENDS
    "${SIV3D_SRC}/Siv3D/*.cpp"
)

# Common-side TUs that include a per-platform detail header (C*.hpp /
# *Detail.hpp) which has no Linux implementation yet. Excluded on Linux only;
# the rest of each subsystem still compiles. See TODO(linux).
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
    # TODO(linux): macOS_Linux holds only POSIX-shared utility code (FileSystem,
    #   Logger, Keyboard ...). There is no Window/Renderer/Texture backend for Linux.
    #   This builds the common library but will NOT link a runnable engine until an
    #   OpenGL/Vulkan backend is implemented under Siv3D-Platform/Linux/.
    file(GLOB_RECURSE SIV3D_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux/*.cpp"
    )
    # These macOS_Linux sources need the GLFW layer, which currently lives under
    # Siv3D-Platform/macOS/ only; they move back in when a Linux backend exists.
    list(FILTER SIV3D_PLATFORM_SOURCES EXCLUDE REGEX "/macOS_Linux/Siv3D/(CursorStyle|Keyboard|Mouse)/")
    message(WARNING "Siv3D: Linux has no graphics backend yet — building common code only (not a linkable runtime). See TODO(linux).")
endif()

# ============================================================================
# Siv3D static library target
# ============================================================================
add_library(Siv3D STATIC
    ${SIV3D_COMMON_SOURCES}
    ${SIV3D_PLATFORM_SOURCES}
    ${SIV3D_TP_SOURCES}
)
add_library(Siv3D::Siv3D ALIAS Siv3D)

# C++23 as a usage requirement: PUBLIC so consumers that include Siv3D's headers
# (e.g. <Siv3D/SIMDMath.hpp>) inherit the standard. Disable compiler extensions
# to keep portable -std=c++23 (not gnu++23). OBJCXX is set in parallel for the
# Apple .mm sources, which target_compile_features(cxx_std_23) does not cover.
target_compile_features(Siv3D PUBLIC cxx_std_23)

# Debug postfix lets `cmake --install` of both configurations share one prefix
# (Siv3Dd.lib / Siv3D.lib), and the exported per-config locations pick the
# matching one in the consumer.
set_target_properties(Siv3D PROPERTIES CXX_EXTENSIONS OFF DEBUG_POSTFIX d)

# Mirrors the vcxproj AdditionalOptions. PUBLIC: the public headers contain
# UTF-8 text, and consumers must compile them with the same source charset
# and a truthful __cplusplus. Guarded by a genex (not if(MSVC)) so the flags
# stay correctly conditional inside the exported package interface.
target_compile_options(Siv3D PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8;/Zc:__cplusplus;/MP>")
if(APPLE)
    set_target_properties(Siv3D PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
endif()

# Include dirs mirror the vcxproj/Xcode IncludePath. include/ThirdParty is PUBLIC
# because public headers (e.g. Int128.hpp -> absl int128.h) include sibling files
# as "absl/..." — without this dir those resolve into vcpkg's absl and mix versions.
# PUBLIC dirs are split per interface: the build tree uses the source layout,
# the installed package uses include/Siv3D (see the install section below).
target_include_directories(Siv3D
    PUBLIC
        "$<BUILD_INTERFACE:${SIV3D_INCLUDE}>"            # <Siv3D/...> public headers
        "$<BUILD_INTERFACE:${SIV3D_INCLUDE}/ThirdParty>" # vendored public deps (absl, fmt, ...)
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D/ThirdParty>"
    PRIVATE
        "${SIV3D_SRC}"                # internal headers + vendored <ThirdParty/...>
        "${SIV3D_SRC}/ThirdParty"     # vendored libs that include themselves bare (serial_cpp, ...)
        "${SIV3D_SRC}/ThirdParty/skia" # skia headers use repo-root-relative "include/core/..."
)
if(WIN32)
    target_include_directories(Siv3D PRIVATE "${SIV3D_SRC}/Siv3D-Platform/WindowsDesktop")
elseif(APPLE)
    target_include_directories(Siv3D PRIVATE
        "${SIV3D_SRC}/Siv3D-Platform/macOS"
        "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux"
    )
elseif(UNIX)
    target_include_directories(Siv3D PRIVATE "${SIV3D_SRC}/Siv3D-Platform/macOS_Linux")
endif()

# Definitions shared by the vcxproj and Xcode builds. _UNICODE is required on all
# platforms: vendored muparser switches mu::string_type to std::wstring on it, and
# the MathParser engine code assumes the wide variant.
target_compile_definitions(Siv3D PRIVATE
    SIV3D_LIBRARY_BUILD
    ZSTD_MULTITHREAD
    ONIG_STATIC
    MUPARSER_STATIC
    SK_RELEASE
    SK_ENABLE_OPTIMIZE_SIZE
    SK_DISABLE_TRACING
    SK_DISABLE_EFFECT_DESERIALIZATION
    LUNASVG_BUILD_STATIC
    OSC_HOST_LITTLE_ENDIAN
    R8B_PFFFT_DOUBLE
    TOML11_COMPILE_SOURCES
    _UNICODE
)
if(MSVC)
    target_compile_definitions(Siv3D PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
    )
endif()

target_link_libraries(Siv3D PRIVATE ${SIV3D_VCPKG_LIBS})

# CMake consumers receive the engine and its third-party libraries as imported
# targets, so the MSBuild-oriented auto-link pragmas in Windows/Libraries.hpp
# must not fire (they name the prebuilt Siv3D/lib layout). The system-library
# pragmas in that header stay active either way.
target_compile_definitions(Siv3D INTERFACE SIV3D_NO_AUTOLINK)

# ---- SIMD baseline (SSE4.2) -----------------------------------------------
# Siv3D requires SSE4.2: the engine uses SSE4.2 intrinsics (<nmmintrin.h>, guarded
# by __SSE4_2__) and the PUBLIC SIMD math headers (Siv3D/SIMDMath.hpp, FastMath.ipp,
# DirectXMath) use SSE intrinsics up to SSE4.1, so on GCC/Clang the code fails to
# compile without -msse4.2 (which also enables SSE4.1 and below). The flag is PUBLIC
# so consumers that include those headers get it too. x86-64 only: MSVC enables these
# intrinsics unconditionally, and ARM (e.g. arm64-osx) takes the NEON path instead.
# AVX-using sources (skia, simdutf) dispatch at runtime via per-function targets,
# so we intentionally do NOT raise the global baseline above SSE4.2.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    target_compile_options(Siv3D PUBLIC "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse4.2>")
endif()

# ---- platform system libraries --------------------------------------------
if(WIN32)
    target_link_libraries(Siv3D PRIVATE
        d3d11 dxgi dxguid d3dcompiler
        winmm ws2_32 imm32 version
    )
    target_compile_definitions(Siv3D PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE)
elseif(APPLE)
    find_library(COCOA_LIB    Cocoa     REQUIRED)
    find_library(METAL_LIB    Metal     REQUIRED)
    find_library(METALKIT_LIB MetalKit  REQUIRED)
    find_library(QUARTZ_LIB   QuartzCore REQUIRED)
    find_library(IOKIT_LIB    IOKit     REQUIRED)
    target_link_libraries(Siv3D PRIVATE
        ${COCOA_LIB} ${METAL_LIB} ${METALKIT_LIB} ${QUARTZ_LIB} ${IOKIT_LIB}
    )
elseif(UNIX)
    # glfw3 comes from vcpkg on non-Windows; OpenGL/X11/Wayland TBD with the backend.
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(Siv3D PRIVATE glfw)
endif()

# Vendored zstd is built with ZSTD_MULTITHREAD; consumers must link pthread on POSIX.
if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(Siv3D PUBLIC Threads::Threads)
endif()

# ============================================================================
# Install / package export
#   Applications consume the engine with find_package(Siv3D CONFIG) and link
#   Siv3D::Siv3D. Two ways to point an app at the package:
#     - build tree (no install step):  -DSiv3D_DIR=<this build dir>
#     - installed prefix:  cmake --install <build> --config Release --prefix <p>
#       (repeat with --config Debug to install both), then -DCMAKE_PREFIX_PATH=<p>
# ============================================================================
include(CMakePackageConfigHelpers)

install(TARGETS Siv3D EXPORT Siv3DTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

# The whole public header tree, include/ThirdParty included (public headers
# depend on it). Nested under include/Siv3D so that a shared install prefix
# does not collide with the real absl/fmt/... packages in include/.
install(DIRECTORY "${SIV3D_INCLUDE}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")

set(SIV3D_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Siv3D")

install(EXPORT Siv3DTargets
    NAMESPACE Siv3D::
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}"
)

# Build-tree twin of the installed export so Siv3D_DIR can point at this build
# directory directly during development.
export(EXPORT Siv3DTargets
    NAMESPACE Siv3D::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/Siv3DTargets.cmake"
)

# The config only resolves dependencies and includes Siv3DTargets.cmake next to
# itself, so the one generated file serves both the build tree and the install.
configure_package_config_file("cmake/Siv3DConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake"
    INSTALL_DESTINATION "${SIV3D_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    COMPATIBILITY SameMinorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}"
)

# ============================================================================
# Optional: unit tests (requires `vcpkg install` with the 'tests' feature)
# ============================================================================
if(SIV3D_BUILD_TESTS)
    find_package(doctest CONFIG REQUIRED)
    file(GLOB_RECURSE SIV3D_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    if(SIV3D_TEST_SOURCES)
        add_executable(Siv3D-Test ${SIV3D_TEST_SOURCES})
        target_link_libraries(Siv3D-Test PRIVATE Siv3D::Siv3D doctest::doctest)
    endif()
endif()
