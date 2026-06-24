vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Reputeless/EnumBitmask
    REF 6a7fac5bc1f03129d2191ca85e089f232a658063
    SHA512 262380fbdb4fa236591a285e9bbbe17c9a6e57287f6ec53386a47a6923f4a74cb9748e1a98caa57e0c2fd439cff90165d055c5f3db4e050a5bf3c384c1e57761
    HEAD_REF main
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-enumbitmask CONFIG_PATH share/unofficial-enumbitmask)

set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
