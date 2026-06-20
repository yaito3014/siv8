cmake_minimum_required(VERSION 3.25)

# ============================================================================
# Standalone Siv3D application project (CMake port of Siv3D-Test.vcxproj).
# The engine is consumed ONLY through find_package(Siv3D); point CMake at one of
#   - the engine build tree:   cmake -B build -DSiv3D_DIR=<repo>/build
#   - an installed prefix:     cmake -B build -DCMAKE_PREFIX_PATH=<prefix>
#
# Siv3D is a static library whose package config find_dependency()s its vcpkg
# dependencies, so this project also configures with the vcpkg toolchain (which
# provides the per-port wrapper machinery the configs rely on) and reuses the
# engine build's vcpkg_installed tree — no second install. Has no manifest of
# its own, so the toolchain runs in classic mode (resolves nothing, just wires
# up the prefix).
# ============================================================================
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

# Must match the engine build: static triplet + static CRT (/MT, /MTd).
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg triplet")
endif()
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Reuse the engine build's installed dependency tree (configs for every dep,
# including the overlay forks) instead of installing a second copy.
if(WIN32)
    set(_siv3d_app_dep_triplet "x64-windows-static")
elseif(APPLE)
    set(_siv3d_app_dep_triplet "x64-osx")
else()
    set(_siv3d_app_dep_triplet "x64-linux")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../build/vcpkg_installed/${_siv3d_app_dep_triplet}")
    list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/vcpkg_installed/${_siv3d_app_dep_triplet}")
endif()

project(Siv3D-App LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

# A user-wide `vcpkg integrate install` would inject the classic-mode tree
# (include path + autolinked libs) into this MSBuild project, shadowing the
# engine's vendored ThirdParty headers (absl version mismatch -> unresolved
# externals). All dependencies come through find_package(Siv3D) instead.
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

find_package(Siv3D 0.8 CONFIG REQUIRED)

option(SIV3D_APP_BUILD_TESTS "Compile ../Test into the app so RunTest() is available" ON)

set(SIV3D_APP_SOURCES Main.cpp)
if(SIV3D_APP_BUILD_TESTS)
    file(GLOB SIV3D_APP_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../Test/*.cpp")
    list(APPEND SIV3D_APP_SOURCES ${SIV3D_APP_TEST_SOURCES})
endif()

add_executable(Siv3D-App ${SIV3D_APP_SOURCES})
target_link_libraries(Siv3D-App PRIVATE Siv3D::Siv3DCore)

# Same build-throughput trick as the vcxproj (pch.h just includes <Siv3D.hpp>).
target_precompile_headers(Siv3D-App PRIVATE pch.h)

if(WIN32)
    # WinMain() is provided by the Siv3D library; the app is a GUI-subsystem exe.
    set_target_properties(Siv3D-App PROPERTIES WIN32_EXECUTABLE ON)
    # Engine resources (icon + optional embedded assets) and the manifest that
    # switches the process code page to UTF-8.
    target_sources(Siv3D-App PRIVATE App/Resource.rc codepage.manifest)
endif()

if(MSVC)
    target_compile_definitions(Siv3D-App PRIVATE
        _WINDOWS
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
    )
endif()

# F5 in Visual Studio runs from App/ (engine/, example/ assets), like the
# vcxproj. Outside the IDE, run the exe with App/ as the working directory.
set_target_properties(Siv3D-App PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/App"
    VS_DEBUGGER_ENVIRONMENT "SIV3D_APP_LAUNCHED_FROM_IDE=1"
)

# Outside a debugger the engine chdirs to the exe's own directory at startup
# (SetWorkingDirectory in Siv3DMainHelper.cpp), so stage a runnable copy in
# App/ — the CMake equivalent of the vcxproj's post-build xcopy. With the
# static triplet the exe is self-contained (any *.dll the script picks up
# would come from a dynamic-triplet build). App/ holds the most recently
# built configuration.
add_custom_command(TARGET Siv3D-App POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DEXE=$<TARGET_FILE:Siv3D-App>
        -DSRC_DIR=$<TARGET_FILE_DIR:Siv3D-App>
        -DDST_DIR=${CMAKE_CURRENT_SOURCE_DIR}/App
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAppBinaries.cmake"
    COMMENT "Staging Siv3D-App and runtime DLLs into App/"
)
