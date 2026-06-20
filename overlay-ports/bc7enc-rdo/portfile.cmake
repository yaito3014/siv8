# Siv3D overlay port for richgel999/bc7enc_rdo.
#
# Provenance:  REF below is the exact upstream commit (a real fork would point
#              REPO at the fork instead). Any local change is an explicit,
#              reviewable .patch in PATCHES — never a silent edit to bundled
#              source. That is the whole point of this port over bare vendoring.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO richgel999/bc7enc_rdo
    REF dbe416d28a5530b4e8cc45b14bf034dc6b96bbde
    SHA512 eb247d4e2c75dab592f6122c664f256fcc81fad0b321e14ae3e750986d40c7d230626ec15af209ce1282b704ad99d28c0994aadc2decdd8196c3b8a02f097e39
    HEAD_REF master
    PATCHES
        0001-ert-include-cstdint.patch
)

# Upstream has no build system and bundles a CLI (test.cpp) plus PNG/zip helpers
# (lodepng, miniz). Supply a CMakeLists that builds ONLY the core encoder TUs
# the engine uses; SUPPORT_BC7E stays undefined (no ISPC kernel header).
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/build-CMakeLists.txt"
    "${SOURCE_PATH}/CMakeLists.txt"
    COPYONLY)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-bc7enc-rdo CONFIG_PATH share/unofficial-bc7enc-rdo)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# bc7enc_rdo's files carry MIT / public-domain headers; record provenance.
file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright"
    "bc7enc_rdo by Rich Geldreich — MIT License.\nUpstream: https://github.com/richgel999/bc7enc_rdo\nPinned commit: dbe416d28a5530b4e8cc45b14bf034dc6b96bbde\nLocal patch: ert.h adds <cstdint> (see 0001-ert-include-cstdint.patch).\n")
