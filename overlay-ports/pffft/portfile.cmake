vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# marton78/pffft — the "PFFFT DOUBLE" source per Siv3D's ThirdParty.md (the
# float-only registry port is jpommier/pffft). Provides pffft_double.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO marton78/pffft
    REF a4b03590cc2a4bea56f9721996e3057835799179
    SHA512 0829120df450534bf1fa7a179962415f46cf697909cbd87a130499ed10e72e00fd7456a1750f1928dfdca3b6f77d77a8f62e01937b8e241d3ee5ee87fc9262fe
    HEAD_REF master
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS_DEBUG
        -DDISABLE_INSTALL_HEADERS=ON
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
