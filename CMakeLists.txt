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
# The same list generates the find_dependency calls in
# Siv3DVariantConfig.cmake: the exported link interface of a static library
# keeps its PRIVATE dependencies (as $<LINK_ONLY:...>), so consumers need every
# package found here.
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
    # Headers only; the PDF renderer loads the shared library at run time.
    "unofficial-pdfium-binaries CONFIG"
)
if(WIN32)
    list(APPEND SIV3D_DEPENDENCIES "unofficial-wintoast CONFIG")
else()
    list(APPEND SIV3D_DEPENDENCIES "glfw3 CONFIG" "Threads")
    if(NOT APPLE)
        # OpenGL 4.1 backend: glad (GL loader) + system GL; Xlib for GLFW.
        list(APPEND SIV3D_DEPENDENCIES "glad CONFIG" "OpenGL" "X11")
    endif()
endif()

set(_siv3d_find_dependency_calls "")
foreach(_siv3d_dep IN LISTS SIV3D_DEPENDENCIES)
    separate_arguments(_siv3d_dep_args UNIX_COMMAND "${_siv3d_dep}")
    find_package(${_siv3d_dep_args} REQUIRED)
    list(APPEND _siv3d_find_dependency_calls "    find_dependency(${_siv3d_dep})")
endforeach()
list(JOIN _siv3d_find_dependency_calls "\n" SIV3D_FIND_DEPENDENCY_CALLS)

# The vcpkg tree in use, with forward slashes, to tell its debug libraries
# (debug/lib) from its release ones (lib).
file(TO_CMAKE_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" SIV3D_VCPKG_TRIPLET_DIR)

# oscpack (OSC networking) comes from vcpkg (formerly vendored in ThirdParty/).
# The port exports no CMake config, so locate the headers and both builds of the
# static lib directly; a release-only triplet has no debug one.
find_path(OSCPACK_INCLUDE_DIR "osc/OscOutboundPacketStream.h" REQUIRED)
find_library(OSCPACK_LIBRARY_RELEASE oscpack
    PATHS "${SIV3D_VCPKG_TRIPLET_DIR}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(OSCPACK_LIBRARY_DEBUG oscpack
    PATHS "${SIV3D_VCPKG_TRIPLET_DIR}/debug/lib" NO_DEFAULT_PATH)
if(NOT OSCPACK_LIBRARY_DEBUG)
    set(OSCPACK_LIBRARY_DEBUG "${OSCPACK_LIBRARY_RELEASE}")
endif()

# Fail early on a triplet with dynamic libraries (the static targets are absent).
if(NOT TARGET mimalloc-static OR NOT TARGET zstd::libzstd_static)
    message(FATAL_ERROR
        "Siv3D requires a vcpkg triplet with static libraries (got '${VCPKG_TARGET_TRIPLET}'): "
        "<arch>-windows-static (/MT) or <arch>-windows-static-md (/MD) on Windows, "
        "x64-linux, x64-osx or arm64-osx elsewhere.")
endif()

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
# Siv3DVariantConfig.cmake gets the same lists, so keep one option per genex: each
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
        libjpeg-turbo::turbojpeg-static
        unofficial::bc7enc-rdo::bc7enc_rdo
        unofficial::levenshtein-sse::levenshtein-sse
        unofficial::rectpack2d::rectpack2d
        unofficial::easyexif::easyexif
        unofficial::r8brain-free-src::r8brain-free-src
        serial_cpp::serial_cpp
        unofficial::pdfium-binaries::headers
)

# oscpack: no exported target, so wire the located headers/lib by hand. On
# Windows it needs Winsock (ws2_32) and winmm, both linked below.
target_include_directories(Siv3DCore PRIVATE ${OSCPACK_INCLUDE_DIR})
target_link_libraries(Siv3DCore PRIVATE
    "$<$<CONFIG:Debug>:${OSCPACK_LIBRARY_DEBUG}>"
    "$<$<NOT:$<CONFIG:Debug>>:${OSCPACK_LIBRARY_RELEASE}>")

# Platform system libraries.
if(WIN32)
    target_link_libraries(Siv3DCore PRIVATE
        d3d11 dxgi dxguid d3dcompiler winmm ws2_32 imm32 version
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
    # libX11 only: X11_LIBRARIES also holds libXext, which nothing here calls.
    target_link_libraries(Siv3DCore PRIVATE X11::X11)
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

# Sets <out_var> to the name a consumer links the system library at <path> by,
# so the installed package does not depend on this machine's SDK paths.
function(_siv3d_system_library_name _path _out_var)
    get_filename_component(_file "${_path}" NAME)
    if(_file MATCHES "^(.+)\\.framework$")
        set(_name "-framework ${CMAKE_MATCH_1}")
    elseif(_file MATCHES "^(.+)\\.[Ll][Ii][Bb]$")
        set(_name "${CMAKE_MATCH_1}")
    elseif(_file MATCHES "^lib(.+)\\.(a|so|dylib|tbd)$")
        set(_name "${CMAKE_MATCH_1}")
    else()
        set(_name "${_path}")
    endif()
    set(${_out_var} "${_name}" PARENT_SCOPE)
endfunction()

# Walks the engine's transitive link closure for one build configuration (DEBUG
# or RELEASE). <libs_var> gets the vcpkg archives the fat lib bundles: only what
# the engine links, not every archive vcpkg built (e.g. skottie/svg/sksg are
# never linked). <system_var> gets everything else the closure links (system
# libraries, as names, and flags), which the fat lib leaves to its link
# interface. Imported targets give their location for the configuration. Raw
# library paths (oscpack, or brotli/bz2/zstd/libpng from dependencies' link
# interfaces) come in debug/release pairs, inside config generator expressions
# or after optimized/debug keywords; instead of evaluating those, each vcpkg path
# is assigned by its directory (<triplet>/debug/lib is DEBUG, <triplet>/lib is
# RELEASE).
function(_siv3d_fat_closure _root _config _libs_var _system_var)
    string(TOLOWER "${SIV3D_VCPKG_TRIPLET_DIR}/" _vcpkg)
    if(_config STREQUAL "DEBUG")
        set(_locations DEBUG)
        set(_excluded "${_vcpkg}lib/")
    else()
        set(_locations RELEASE RELWITHDEBINFO MINSIZEREL)
        set(_excluded "${_vcpkg}debug/lib/")
    endif()
    set(_seen "")
    set(_libs "")
    set(_system "")
    set(_stack "${_root}")
    while(_stack)
        list(POP_BACK _stack _raw)
        if(_raw MATCHES "^\\$<LINK_ONLY:([^$<>]+)>$")
            set(_item "${CMAKE_MATCH_1}")
        elseif(_raw MATCHES "\\$<|>$")
            # A config generator expression, possibly cut at a ';' of its list
            # (its head holds "$<", its tail ends in ">"): keep the library path
            # in it, if any. A drive letter must not end a word ($<BOOL:/usr/...>).
            if(_raw MATCHES "(^|[^A-Za-z])(([A-Za-z]:)?/[^$<>;]*\\.([Aa]|[Ll][Ii][Bb]))")
                set(_item "${CMAKE_MATCH_2}")
            else()
                continue()
            endif()
        else()
            set(_item "${_raw}")
        endif()
        string(STRIP "${_item}" _item)
        # Skip the keywords of optimized/debug pairs (the paths after them are
        # sorted by directory) and the ::@ markers CMake adds to link items.
        if(_item STREQUAL "" OR _item IN_LIST _seen
                OR _item MATCHES "^(optimized|debug|general)$" OR _item MATCHES "^::@")
            continue()
        endif()
        list(APPEND _seen "${_item}")
        if(_item STREQUAL "Threads::Threads")
            # Kept as the target: the package config finds Threads again.
            list(APPEND _system "${_item}")
            continue()
        endif()
        if(NOT TARGET "${_item}")
            if(IS_ABSOLUTE "${_item}")
                set(_loc "${_item}")
            else()
                # A bare library name or a linker flag.
                list(APPEND _system "${_item}")
                continue()
            endif()
        else()
            set(_loc "")
            get_target_property(_type "${_item}" TYPE)
            get_target_property(_imp "${_item}" IMPORTED)
            if(_imp AND _type MATCHES "^(STATIC|SHARED|UNKNOWN)_LIBRARY$")
                foreach(_c IN LISTS _locations)
                    get_target_property(_try "${_item}" IMPORTED_LOCATION_${_c})
                    if(_try)
                        set(_loc "${_try}")
                        break()
                    endif()
                endforeach()
                if(NOT _loc)
                    get_target_property(_loc "${_item}" IMPORTED_LOCATION)
                endif()
            endif()
            foreach(_prop INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
                get_target_property(_deps "${_item}" ${_prop})
                if(_deps)
                    list(APPEND _stack ${_deps})
                endif()
            endforeach()
            if(NOT _loc)
                continue()
            endif()
        endif()
        file(TO_CMAKE_PATH "${_loc}" _path)
        string(TOLOWER "${_path}" _lower)
        string(FIND "${_lower}" "${_vcpkg}" _at)
        if(_at EQUAL 0)
            # An imported target's location is already the configuration's;
            # a raw path may be the other configuration's build.
            string(FIND "${_lower}" "${_excluded}" _at)
            if(_lower MATCHES "\\.(a|lib)$" AND EXISTS "${_path}"
                    AND (TARGET "${_item}" OR NOT _at EQUAL 0))
                list(APPEND _libs "${_path}")
            endif()
        else()
            _siv3d_system_library_name("${_path}" _name)
            list(APPEND _system "${_name}")
        endif()
    endwhile()
    list(REMOVE_DUPLICATES _libs)
    list(REMOVE_DUPLICATES _system)
    set(${_libs_var} "${_libs}" PARENT_SCOPE)
    set(${_system_var} "${_system}" PARENT_SCOPE)
endfunction()

# ============================================================================
# Fat library: libSiv3D.a — engine + every dependency it links, content-deduped.
# Default ON for a top-level build, OFF under add_subdirectory.
# ============================================================================
option(SIV3D_FAT_LIB "Build the fat libSiv3D.a bundling all deps" ${PROJECT_IS_TOP_LEVEL})
if(SIV3D_FAT_LIB)
    # One fat lib per configuration: Debug bundles the debug builds of the
    # dependencies (with MSVC, those of the debug C runtime), the others the
    # release ones. Both lists are fixed at configure time, whatever the
    # generator's configurations, and must pair up one to one.
    _siv3d_fat_closure(Siv3DCore RELEASE _siv3d_fatdeps_release _siv3d_fat_system)
    if(IS_DIRECTORY "${SIV3D_VCPKG_TRIPLET_DIR}/debug/lib")
        _siv3d_fat_closure(Siv3DCore DEBUG _siv3d_fatdeps_debug _siv3d_fat_system_debug)
        list(APPEND _siv3d_fat_system ${_siv3d_fat_system_debug})
        list(REMOVE_DUPLICATES _siv3d_fat_system)
    else()
        # A release-only triplet has no debug builds to bundle.
        set(_siv3d_fatdeps_debug "${_siv3d_fatdeps_release}")
    endif()
    list(LENGTH _siv3d_fatdeps_release _siv3d_fatdeps_n)
    list(LENGTH _siv3d_fatdeps_debug _siv3d_fatdeps_debug_n)
    if(NOT _siv3d_fatdeps_n EQUAL _siv3d_fatdeps_debug_n)
        string(REPLACE ";" "\n  " _siv3d_r "${_siv3d_fatdeps_release}")
        string(REPLACE ";" "\n  " _siv3d_d "${_siv3d_fatdeps_debug}")
        message(FATAL_ERROR "Siv3D fat lib: ${_siv3d_fatdeps_n} release but "
            "${_siv3d_fatdeps_debug_n} debug dependency archives; a dependency "
            "lacks one of its builds.\nRelease:\n  ${_siv3d_r}\nDebug:\n  ${_siv3d_d}")
    endif()
    message(STATUS "Siv3D fat lib: bundling ${_siv3d_fatdeps_n} dependency archives per configuration (engine link closure)")
    message(STATUS "Siv3D fat lib: system libraries: ${_siv3d_fat_system}")

    # A directory per configuration: Release, RelWithDebInfo and MinSizeRel share
    # the file name, and a multi-config generator needs distinct outputs.
    set(_siv3d_fat "${CMAKE_BINARY_DIR}/fatlib/$<CONFIG>/${CMAKE_STATIC_LIBRARY_PREFIX}Siv3D$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}")
    if(MSVC)
        # lib.exe merges (deduping by symbol, not content) from a response file of
        # the closure archives — quoted, one per line, to dodge cmd length limits.
        foreach(_siv3d_cfg release debug)
            set(_siv3d_rsp "")
            foreach(_l IN LISTS _siv3d_fatdeps_${_siv3d_cfg})
                string(APPEND _siv3d_rsp "\"${_l}\"\n")
            endforeach()
            file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/_fatlib_inputs-${_siv3d_cfg}.rsp" CONTENT "${_siv3d_rsp}")
        endforeach()
        set(_siv3d_inputs "${CMAKE_BINARY_DIR}/_fatlib_inputs-$<IF:$<CONFIG:Debug>,debug,release>.rsp")
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND lib.exe /NOLOGO "/OUT:${_siv3d_fat}" "$<TARGET_FILE:Siv3DCore>" "@${_siv3d_inputs}"
            DEPENDS Siv3DCore "${_siv3d_inputs}" VERBATIM)
    else()
        foreach(_siv3d_cfg release debug)
            string(REPLACE ";" "\n" _siv3d_fatdeps_nl "${_siv3d_fatdeps_${_siv3d_cfg}}")
            file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/_fatlib_inputs-${_siv3d_cfg}.txt" CONTENT "${_siv3d_fatdeps_nl}\n")
        endforeach()
        set(_siv3d_inputs "${CMAKE_BINARY_DIR}/_fatlib_inputs-$<IF:$<CONFIG:Debug>,debug,release>.txt")
        add_custom_command(OUTPUT "${_siv3d_fat}"
            COMMAND "${CMAKE_COMMAND}"
                "-DSIV3D_MAIN_LIB=$<TARGET_FILE:Siv3DCore>"
                "-DSIV3D_INPUTS_FILE=${_siv3d_inputs}"
                "-DSIV3D_FATLIB_OUT=${_siv3d_fat}"
                "-DSIV3D_AR=${CMAKE_AR}" "-DSIV3D_RANLIB=${CMAKE_RANLIB}"
                "-DSIV3D_WORK=${CMAKE_BINARY_DIR}/_fatlib/$<CONFIG>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake"
            DEPENDS Siv3DCore "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DFatLib.cmake" "${_siv3d_inputs}"
            COMMENT "Building deduped fat static lib: ${_siv3d_fat}" VERBATIM)
    endif()
    add_custom_target(Siv3D-fat ALL DEPENDS "${_siv3d_fat}")
endif()

# ============================================================================
# Install: fat libSiv3D.a + headers (usable from any build system, or CMake).
# ============================================================================
# The C runtime of this build, exported so consumers can match it (empty
# outside MSVC). A subproject may leave CMAKE_MSVC_RUNTIME_LIBRARY unset, i.e.
# CMake's default.
set(SIV3D_MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
if(MSVC AND NOT SIV3D_MSVC_RUNTIME_LIBRARY)
    set(SIV3D_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# With MSVC, the libraries and the CMake files of a build go to a subdirectory
# named after its C runtime kind (mt: static, md: DLL), so builds for both can
# be installed into one prefix; cmake/Siv3DConfig.cmake picks between them.
# Headers are the same for both and stay shared.
set(_siv3d_variant_subdir "")
if(MSVC)
    siv3d_msvc_runtime_kind("${SIV3D_MSVC_RUNTIME_LIBRARY}" _siv3d_crt_kind)
    if(_siv3d_crt_kind STREQUAL "static")
        set(_siv3d_variant_subdir "/mt")
    else()
        set(_siv3d_variant_subdir "/md")
    endif()
endif()
set(SIV3D_INSTALL_VARIANT_LIBDIR "${CMAKE_INSTALL_LIBDIR}${_siv3d_variant_subdir}")
set(SIV3D_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Siv3D")
set(SIV3D_INSTALL_VARIANT_CMAKEDIR "${SIV3D_INSTALL_CMAKEDIR}${_siv3d_variant_subdir}")

if(SIV3D_FAT_LIB)
    install(FILES "${_siv3d_fat}" DESTINATION "${SIV3D_INSTALL_VARIANT_LIBDIR}")
endif()

# The engine loads PDFium at run time (engine/dll/pdfium.dll under the working
# directory on Windows, Contents/Frameworks/dll/libpdfium.dylib in a macOS
# bundle), so ship the library and its licenses for consumers to stage there.
if(WIN32 OR APPLE)
    if(WIN32)
        set(_siv3d_pdfium_dir "${CMAKE_INSTALL_BINDIR}")
    else()
        set(_siv3d_pdfium_dir "${CMAKE_INSTALL_LIBDIR}")
    endif()
    install(FILES "$<TARGET_FILE:unofficial::pdfium-binaries::library>"
        DESTINATION "${_siv3d_pdfium_dir}")
    install(FILES "${unofficial-pdfium-binaries_LICENSE}"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/Siv3D/licenses" RENAME "pdfium.txt")
endif()

# Third-party headers the public Siv3D headers include go to a sibling
# include/Siv3D-thirdparty (the Siv3D- prefix avoids colliding with system copies
# in a shared prefix), added as a 2nd interface include by Siv3DVariantConfig.cmake.in.
set(SIV3D_THIRDPARTY_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D-thirdparty")
set(_siv3d_depinc "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
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
set(SIV3D_VCPKG_BUNDLE_RELDIR "${CMAKE_INSTALL_DATAROOTDIR}/Siv3D/vcpkg")

# System libraries for the fat target (the raw archive has none of its own usage
# requirements): everything the engine's link closure links besides the bundled
# archives. Its compile options and definitions are Siv3DCore's PUBLIC ones.
if(APPLE)
    # Not derived from the closure on macOS yet, where it is unverified: the
    # engine's framework set.
    set(SIV3D_FAT_SYSTEM_LIBS "")
    foreach(_fw IN LISTS SIV3D_MACOS_FRAMEWORKS)
        list(APPEND SIV3D_FAT_SYSTEM_LIBS "-framework ${_fw}")
    endforeach()
else()
    set(SIV3D_FAT_SYSTEM_LIBS "${_siv3d_fat_system}")
endif()

# The build tree mirrors the installed cmake/Siv3D layout, so -DSiv3D_DIR=<build>
# goes through the same dispatcher.
configure_package_config_file("cmake/Siv3DVariantConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}${_siv3d_variant_subdir}/Siv3DVariantConfig.cmake"
    INSTALL_DESTINATION "${SIV3D_INSTALL_VARIANT_CMAKEDIR}")
configure_file("cmake/Siv3DConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake" COPYONLY)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    COMPATIBILITY SameMinorVersion)

# Config ships in the default install (-> Siv3D::Siv3D).
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Siv3DConfigVersion.cmake"
    DESTINATION "${SIV3D_INSTALL_CMAKEDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}${_siv3d_variant_subdir}/Siv3DVariantConfig.cmake"
    DESTINATION "${SIV3D_INSTALL_VARIANT_CMAKEDIR}")

# 'Develop' component (EXCLUDE_FROM_ALL): core archive + bundle + shim.
#   cmake --install <build> --component Develop
set(_siv3d_cmpt Develop)
# The public headers install by default (the fat lib needs them too), nested
# under include/Siv3D to avoid polluting a shared prefix.
install(TARGETS Siv3DCore EXPORT Siv3DCoreTargets
    ARCHIVE DESTINATION "${SIV3D_INSTALL_VARIANT_LIBDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL
    FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")
install(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    DESTINATION "${SIV3D_INSTALL_VARIANT_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
# The debug builds of the dependencies (~1.7 GB on Linux) ship only when the
# Debug configuration is installed: the debug/ subtree together with the
# *-debug.cmake imported-config files, which set IMPORTED_LOCATION_DEBUG to those
# libs and would make find_package fail ("not all the files it references")
# without them.
set(_siv3d_bundle "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(_siv3d_bundle_dest "${SIV3D_VCPKG_BUNDLE_RELDIR}/${VCPKG_TARGET_TRIPLET}")
install(DIRECTORY "${_siv3d_bundle}/"
    DESTINATION "${_siv3d_bundle_dest}"
    COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL
    PATTERN "debug" EXCLUDE
    PATTERN "*-debug.cmake" EXCLUDE)
if(IS_DIRECTORY "${_siv3d_bundle}/debug")
    install(DIRECTORY "${_siv3d_bundle}/debug/"
        DESTINATION "${_siv3d_bundle_dest}/debug"
        CONFIGURATIONS Debug
        COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)
    install(DIRECTORY "${_siv3d_bundle}/share/"
        DESTINATION "${_siv3d_bundle_dest}/share"
        CONFIGURATIONS Debug
        COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL
        FILES_MATCHING PATTERN "*-debug.cmake")
endif()

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
    # are ~440 MB of dead weight in the release lib/ on Windows (and again in
    # debug/lib). Exact basenames so 'svg' doesn't clip lunasvg.
    foreach(_siv3d_l skottie svg sksg)
        file(REMOVE \"\${_siv3d_tp}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}\${_siv3d_l}${CMAKE_STATIC_LIBRARY_SUFFIX}\"
            \"\${_siv3d_tp}/debug/lib/${CMAKE_STATIC_LIBRARY_PREFIX}\${_siv3d_l}${CMAKE_STATIC_LIBRARY_SUFFIX}\")
    endforeach()
" COMPONENT ${_siv3d_cmpt})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DVcpkgShim.cmake"
    DESTINATION "${SIV3D_INSTALL_VARIANT_CMAKEDIR}" COMPONENT ${_siv3d_cmpt} EXCLUDE_FROM_ALL)

# Build-tree export for -DSiv3D_DIR=<build>.
export(EXPORT Siv3DCoreTargets NAMESPACE Siv3D::
    FILE "${CMAKE_CURRENT_BINARY_DIR}${_siv3d_variant_subdir}/Siv3DCoreTargets.cmake")

# ============================================================================
# Fat-lib tests (ctest -C <config>, after building that configuration). The
# link test installs the configuration's default component to a prefix in the
# build tree, then builds cmake/FatLibLinkTest against it with the same
# toolchain.
# ============================================================================
if(SIV3D_FAT_LIB AND PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_test(NAME FatLib.bundlesOnlyVcpkgArchives
        COMMAND "${CMAKE_COMMAND}"
            "-DSIV3D_INPUTS_FILE=${_siv3d_inputs}"
            "-DSIV3D_VCPKG_TRIPLET_DIR=${SIV3D_VCPKG_TRIPLET_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Siv3DCheckFatLibInputs.cmake")

    set(_siv3d_flt "${CMAKE_CURRENT_BINARY_DIR}/FatLibLinkTest/$<CONFIG>")
    add_test(NAME FatLib.install
        COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_CURRENT_BINARY_DIR}"
            --config $<CONFIG> --prefix "${_siv3d_flt}/prefix")
    set_tests_properties(FatLib.install PROPERTIES FIXTURES_SETUP Siv3DFatLibInstalled)

    set(_siv3d_flt_generator --build-generator "${CMAKE_GENERATOR}")
    if(CMAKE_GENERATOR_PLATFORM)
        list(APPEND _siv3d_flt_generator --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}")
    endif()
    if(CMAKE_GENERATOR_TOOLSET)
        list(APPEND _siv3d_flt_generator --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}")
    endif()
    if(CMAKE_MAKE_PROGRAM)
        list(APPEND _siv3d_flt_generator --build-makeprogram "${CMAKE_MAKE_PROGRAM}")
    endif()
    set(_siv3d_flt_options
        "-DCMAKE_BUILD_TYPE=$<CONFIG>"
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DCMAKE_PREFIX_PATH=${_siv3d_flt}/prefix")
    if(MSVC)
        list(APPEND _siv3d_flt_options "-DSIV3D_MSVC_CRT=${_siv3d_crt_kind}")
    endif()
    add_test(NAME FatLib.wholeArchiveLink
        COMMAND "${CMAKE_CTEST_COMMAND}" --build-and-test
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FatLibLinkTest" "${_siv3d_flt}/build"
            ${_siv3d_flt_generator}
            --build-config $<CONFIG>
            --build-options ${_siv3d_flt_options})
    set_tests_properties(FatLib.wholeArchiveLink PROPERTIES FIXTURES_REQUIRED Siv3DFatLibInstalled)
endif()

# ============================================================================
# Tests (vcpkg 'tests' feature)
# ============================================================================
if(SIV3D_BUILD_TESTS)
    find_package(nanobench CONFIG REQUIRED)
    file(GLOB_RECURSE SIV3D_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")
    # Linux/macOS test executable. On Windows it needs the App's embedded engine
    # resources and app/ run directory, so App/CMakeLists.txt defines it there.
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
