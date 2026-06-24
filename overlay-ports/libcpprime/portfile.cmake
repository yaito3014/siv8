vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Rac75116/libcpprime
    REF 19cae2288d0a0c768c169c010bda9b4ee9289e1e
    SHA512 0edc5e1a247969bec663ff0e38eea79560e56c1b1915b5629c3ea4fcc931da61ea710a46433719bc7053c34a8c8034dc91668bc00a49d0479653f210ab8c456a
    HEAD_REF main
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-libcpprime CONFIG_PATH share/unofficial-libcpprime)

set(VCPKG_POLICY_EMPTY_PACKAGE enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
