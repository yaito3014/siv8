vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Reputeless/Xoshiro-cpp
    REF 19bcbb2ce0ed158233187f524fd0964c105a65b3
    SHA512 b708505e3b4e84bdd87fa6d9bde91c95e09eef4b444d5593de331f4ea76d01021a0ca206ebcdfaa3aceaf0dfa161175d8e19b02e62f21ff8e765e2147563a3de
    HEAD_REF master
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-xoshiro-cpp CONFIG_PATH share/unofficial-xoshiro-cpp)

set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
