vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# The exact upstream Siv3D vendored (vendored == this commit, minus one later
# bugfix line). Uses gbionics' own modern CMakeLists (install + config).
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO gbionics/serial_cpp
    REF 2ab9e20388a6efca8540e5ae8910e9e2c42a71f4
    SHA512 2a0fcc263b9aa17198be43e222172799f068d5ff84fd3c23c64e26a25db6e0b9a58a11ab2c8db0b0259bcd3469a8f06d3a63e73b8838bba1315af156003bb1b9
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME serial_cpp CONFIG_PATH lib/cmake/serial_cpp)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
