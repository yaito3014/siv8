vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO adamyaxley/Obfuscate
    REF 5390a353f4e83ffd596730eab0b0e4ac629dbbbc
    SHA512 41233a8c680b4a8f27ccca797c0db8c8a9274169aa1e1521408b9e70c1e88190498f3972426eb57831ac3f5675c40eb8486ebf642a1b95d7dbaf37a9e2d55c78
    HEAD_REF master
)

# Header-only: supply a CMakeLists that builds an INTERFACE target, installs the
# single header, and exports a config (the upstream ships no build system).
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-obfuscate CONFIG_PATH share/unofficial-obfuscate)

set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
