vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO mayanklahiri/easyexif
    REF cd994a3b6009bc3c1f84062e96bd7f5ad16e85f6
    SHA512 f421ff0900ae456165917106caa1c222ed8005706edc62983ad1ea72baffdfc99c581a015cf00132c7255ac32e64ec24bd8d65c9b32e9f4d05809baf7f37516d
    HEAD_REF master
)

# Siv3D overlay: the registry port deletes share/ (no config). Supply a
# CMakeLists that exports a target, and keep its config.
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-easyexif CONFIG_PATH share/unofficial-easyexif)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
