cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

project(Siv3D LANGUAGES CXX VERSION 0.8.0)

if(APPLE)
    enable_language(OBJCXX)
endif()

# The C++ standard is a target usage requirement, set per-target below via
# target_compile_features (not the global CMAKE_CXX_STANDARD), so it stays scoped
# to Siv3D and propagates to consumers through PUBLIC.

# ---- options --------------------------------------------------------------
option(SIV3D_BUILD_TESTS "Build the Siv3D-Test target (needs vcpkg feature 'tests')" OFF)
option(SIV3D_USE_SKIA    "Compile the bundled Skia sources (very heavy)"             OFF)

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
find_package(ZLIB          REQUIRED)                 # ZLIB::ZLIB
find_package(ZSTD CONFIG)                            # zstd::libzstd (optional name varies)
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

# zstd target name differs across configs (static vs shared); pick whatever exists.
if(TARGET zstd::libzstd)
    list(APPEND SIV3D_VCPKG_LIBS zstd::libzstd)
elseif(TARGET zstd::libzstd_static)
    list(APPEND SIV3D_VCPKG_LIBS zstd::libzstd_static)
elseif(TARGET zstd::libzstd_shared)
    list(APPEND SIV3D_VCPKG_LIBS zstd::libzstd_shared)
endif()

# mimalloc target name also varies (static vs shared).
if(TARGET mimalloc-static)
    list(APPEND SIV3D_VCPKG_LIBS mimalloc-static)
elseif(TARGET mimalloc)
    list(APPEND SIV3D_VCPKG_LIBS mimalloc)
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

# Skia (361 files) is opt-in; GLFW is only needed on non-Windows (vcpkg also offers it).
if(NOT SIV3D_USE_SKIA)
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/skia/")
endif()
if(WIN32)
    list(FILTER SIV3D_TP_SOURCES EXCLUDE REGEX "/ThirdParty/GLFW/")
endif()

# ============================================================================
# Engine sources (common + per-platform)
# ============================================================================
file(GLOB_RECURSE SIV3D_COMMON_SOURCES CONFIGURE_DEPENDS
    "${SIV3D_SRC}/Siv3D/*.cpp"
)

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
set_target_properties(Siv3D PROPERTIES CXX_EXTENSIONS OFF)
if(APPLE)
    set_target_properties(Siv3D PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
endif()

target_include_directories(Siv3D
    PUBLIC
        "${SIV3D_INCLUDE}"          # <Siv3D/...> public headers
    PRIVATE
        "${SIV3D_SRC}"              # internal headers + vendored <ThirdParty/...>
)

target_link_libraries(Siv3D PRIVATE ${SIV3D_VCPKG_LIBS})

# ---- SIMD baseline (SSE4.2) -----------------------------------------------
# Siv3D requires SSE4.2: the engine uses SSE4.2 intrinsics (<nmmintrin.h>, guarded
# by __SSE4_2__) and the PUBLIC SIMD math headers (Siv3D/SIMDMath.hpp, FastMath.ipp,
# DirectXMath) use SSE intrinsics up to SSE4.1, so on GCC/Clang the code fails to
# compile without -msse4.2 (which also enables SSE4.1 and below). The flag is PUBLIC
# so consumers that include those headers get it too. x86-64 only: MSVC enables these
# intrinsics unconditionally, and ARM (e.g. arm64-osx) takes the NEON path instead.
# AVX-using sources (skia, simdutf) dispatch at runtime via per-function targets,
# so we intentionally do NOT raise the global baseline above SSE4.2.
if(NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    target_compile_options(Siv3D PUBLIC -msse4.2)
endif()

# ---- platform system libraries --------------------------------------------
if(WIN32)
    target_link_libraries(Siv3D PRIVATE
        d3d11 dxgi dxguid d3dcompiler
        winmm ws2_32 imm32 version
    )
    target_compile_definitions(Siv3D PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
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
