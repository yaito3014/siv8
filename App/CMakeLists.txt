cmake_minimum_required(VERSION 3.25)

# Siv3D sample application — the top-level project. Pulls in the engine via
# either add_subdirectory (default: build in-tree) or find_package
# (-DSIV3D_APP_USE_FIND_PACKAGE=ON: consume an installed/exported Siv3D).
get_filename_component(_siv3d_engine_default "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(SIV3D_ENGINE_DIR "${_siv3d_engine_default}" CACHE PATH "Siv3D engine source directory")
option(SIV3D_APP_USE_FIND_PACKAGE "Consume an installed Siv3D via find_package instead of building it in-tree" OFF)
option(SIV3D_APP_BUILD_TESTS "Compile ../Test into the app so RunTest() is available (needs the engine 'tests' feature)" ON)

# ---- pre-project() setup; must match the engine build ----
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

# Static triplet + static CRT (/MT, /MTd).
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg triplet")
endif()
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(SIV3D_APP_USE_FIND_PACKAGE)
    # Reuse the engine build's installed dependency tree (incl. overlay forks).
    if(WIN32)
        set(_siv3d_app_dep_triplet "x64-windows-static")
    elseif(APPLE)
        set(_siv3d_app_dep_triplet "x64-osx")
    else()
        set(_siv3d_app_dep_triplet "x64-linux")
    endif()
    if(EXISTS "${SIV3D_ENGINE_DIR}/build/vcpkg_installed/${_siv3d_app_dep_triplet}")
        list(PREPEND CMAKE_PREFIX_PATH "${SIV3D_ENGINE_DIR}/build/vcpkg_installed/${_siv3d_app_dep_triplet}")
    endif()
else()
    # Build in-tree: drive vcpkg from the engine's manifest.
    if(NOT DEFINED VCPKG_MANIFEST_DIR)
        set(VCPKG_MANIFEST_DIR "${SIV3D_ENGINE_DIR}" CACHE PATH "Siv3D vcpkg manifest dir")
    endif()
    if(SIV3D_APP_BUILD_TESTS)
        list(APPEND VCPKG_MANIFEST_FEATURES "tests")
    endif()
endif()

project(Siv3D-App LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

# Stop a user-wide `vcpkg integrate install` from shadowing the engine's headers.
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

# ---- pull in the engine: Siv3D::Siv3DCore ----
if(SIV3D_APP_USE_FIND_PACKAGE)
    find_package(Siv3D 0.8 CONFIG REQUIRED)
else()
    # Keep the engine's Siv3D-Test target / 'tests' feature consistent with ours.
    set(SIV3D_BUILD_TESTS ${SIV3D_APP_BUILD_TESTS} CACHE BOOL "Build the Siv3D-Test target" FORCE)
    add_subdirectory("${SIV3D_ENGINE_DIR}" siv3d-engine)
endif()

set(SIV3D_APP_SOURCES Main.cpp)
if(SIV3D_APP_BUILD_TESTS)
    file(GLOB SIV3D_APP_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../Test/*.cpp")
    list(APPEND SIV3D_APP_SOURCES ${SIV3D_APP_TEST_SOURCES})
endif()

add_executable(Siv3D-App ${SIV3D_APP_SOURCES})
target_link_libraries(Siv3D-App PRIVATE Siv3D::Siv3DCore)

# Main.cpp pulls in the test harness only under this define.
if(SIV3D_APP_BUILD_TESTS)
    target_compile_definitions(Siv3D-App PRIVATE SIV3D_APP_BUILD_TESTS)
endif()

# PCH (pch.h just includes <Siv3D.hpp>). MSVC only: GCC double-parses the
# headers (force-included PCH + Main.cpp's own <Siv3D.hpp> -> .ipp redefinitions).
if(MSVC)
	target_precompile_headers(Siv3D-App PRIVATE pch.h)
endif()

if(WIN32)
    # WinMain() is provided by the Siv3D library; the app is a GUI-subsystem exe.
    set_target_properties(Siv3D-App PROPERTIES WIN32_EXECUTABLE ON)
    # Icon/embedded-asset resources + UTF-8 code-page manifest. (Resource.rc
    # references icon.ico by bare name, so they live together.)
    target_sources(Siv3D-App PRIVATE
        platform/WindowsDesktop/Resource.rc
        platform/WindowsDesktop/codepage.manifest)
    # Resource.rc embeds engine/... assets relative to app/; put app/ on the RC
    # include path so rc.exe resolves them.
    target_include_directories(Siv3D-App PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app")
elseif(APPLE)
    # .app bundle (Info.plist + icon). NOTE: staging the runtime assets
    # (app/engine, app/example) and the dylibs into the bundle isn't wired yet —
    # complete and verify on macOS.
    set_target_properties(Siv3D-App PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/macOS/Info.plist"
    )
    target_sources(Siv3D-App PRIVATE platform/macOS/icon.icns)
    set_source_files_properties(platform/macOS/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

if(MSVC)
    target_compile_definitions(Siv3D-App PRIVATE
        _WINDOWS
        _ENABLE_EXTENDED_ALIGNED_STORAGE
        _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
        _SILENCE_CXX23_DENORM_DEPRECATION_WARNING
    )
endif()

# F5 in Visual Studio runs from app/ (the engine/ + example/ assets).
set_target_properties(Siv3D-App PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/app"
    VS_DEBUGGER_ENVIRONMENT "SIV3D_APP_LAUNCHED_FROM_IDE=1"
)

# Stage the exe into app/ (next to the engine/ + example/ assets) so it runs in
# place: App/app/Siv3D-App. (macOS ships a .app bundle instead.)
if(WIN32)
    add_custom_command(TARGET Siv3D-App POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:Siv3D-App>
            -DSRC_DIR=$<TARGET_FILE_DIR:Siv3D-App>
            -DDST_DIR=${CMAKE_CURRENT_SOURCE_DIR}/app
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAppBinaries.cmake"
        COMMENT "Staging Siv3D-App into app/"
    )
elseif(UNIX AND NOT APPLE)
    add_custom_command(TARGET Siv3D-App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Siv3D-App> "${CMAKE_CURRENT_SOURCE_DIR}/app/"
        COMMENT "Staging Siv3D-App into app/"
    )
endif()
