# Minimal example of consuming an installed/exported Siv3D SDK.
#
# This is the canonical downstream-consumer CMakeLists: it does nothing but
# find Siv3D and link it. (The repo's own `App/` builds the engine in-tree for
# development; a real consumer uses the exported SDK exactly like this.)
#
# Configure with the vcpkg toolchain, a matching triplet, and the Siv3D install
# prefix on CMAKE_PREFIX_PATH, e.g.:
#
#   cmake -B build \
#     -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
#     -DVCPKG_TARGET_TRIPLET=x64-linux \
#     -DCMAKE_PREFIX_PATH=<siv3d-install-prefix>
#   cmake --build build
#
cmake_minimum_required(VERSION 3.25)

# Siv3D ships as a static-CRT build, so a consumer must match that ABI.
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
endif()
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(Siv3D-Minimal LANGUAGES CXX)

find_package(Siv3D CONFIG REQUIRED)

add_executable(Siv3D-Minimal Main.cpp)
target_link_libraries(Siv3D-Minimal PRIVATE Siv3D::Siv3D)
