if (VCPKG_TARGET_IS_EMSCRIPTEN)
    # emscripten has built-in glfw3 library
    set(VCPKG_BUILD_TYPE release)
    file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/glfw3Config.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/glfw3")
    set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
    return()
endif()

# Siv3D fork of glfw3 3.5.1: upstream release source, plus
# siv3d-glfw.patch (the GLFW_Siv3D_DragDrop bridge + engine input/cocoa/monitor
# hooks the engine relies on). Pinned by SHA512 so the fork is just
# "upstream 3.5.1 + a reviewable patch", in line with the other overlay forks.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO glfw/glfw
    REF ${VERSION}
    SHA512 42e8b8bdc2ecaab8d33d43cab6cf6e4bfd1f23492a5f13ec4b3762b74661dc3c9625721f436fbf841909b9af3ac6dcb8080201b36592d33c2bb196f768fc307f
    HEAD_REF master
    PATCHES
        siv3d-glfw.patch
)

# The bridge header is a new file the fork adds: the patched cocoa_window.m
# includes it as "GLFW_Siv3D_DragDropBridge.h" (next to it in src/), and the
# engine includes it as <GLFW/GLFW_Siv3D_DragDropBridge.h>. Place it in both.
file(COPY "${CMAKE_CURRENT_LIST_DIR}/GLFW_Siv3D_DragDropBridge.h"
     DESTINATION "${SOURCE_PATH}/src")

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    wayland         GLFW_BUILD_WAYLAND
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
        ${FEATURE_OPTIONS}
    MAYBE_UNUSED_VARIABLES
        GLFW_USE_WAYLAND
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/glfw3)

vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Ship the Siv3D bridge header beside <GLFW/glfw3.h> for the engine to include.
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/GLFW_Siv3D_DragDropBridge.h"
     DESTINATION "${CURRENT_PACKAGES_DIR}/include/GLFW")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
