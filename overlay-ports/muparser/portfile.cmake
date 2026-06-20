# Overlay copy of the registry port `muparser` (vcpkg @ baseline f3e10653),
# with ONE change: -DENABLE_WIDE_CHAR=ON (registry default is OFF). Siv3D's
# MathParser needs the std::wstring build; ENABLE_WIDE_CHAR adds _UNICODE PUBLIC
# to the muparser target, so both the lib and its header are wide for consumers.
# To update: re-copy $VCPKG_ROOT/ports/muparser and re-apply this one flag.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO beltoforion/muparser
    REF "v${VERSION}"
    SHA512 48610dd112b5c8e1ea7615e29c9f9ca185091392b651794de039c14edfad4c62a6ae1d087393fdfd8d03a99f94a6e71275b86ddc8027234d322030bc7c25223e 
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        openmp    ENABLE_OPENMP
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${FEATURE_OPTIONS}
        -DENABLE_SAMPLES=OFF
        -DENABLE_WIDE_CHAR=ON
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/muparser")
vcpkg_fixup_pkgconfig()

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/muParserFixes.h" "#ifndef MUPARSER_STATIC" "#if 0")
else()
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/include/muParserFixes.h" "#ifndef MUPARSER_STATIC" "#if 1")
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
