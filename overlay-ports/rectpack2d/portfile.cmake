# Header-only, pristine. Provenance: pinned upstream commit; the former vendored
# copy matched this commit byte-for-byte, so there is no patch. (It differed from
# upstream HEAD only because HEAD moved on — the exact reason to pin a commit.)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO TeamHypersomnia/rectpack2D
    REF 964fd05997e65c0a1bb90745f85bc0e865ba5de0
    SHA512 607326ce60227d7dabd79aefd329220fe03d1398d0ee3e21661f04cabdc843ecf188a05382d18420163e46a242c48775e7ab104d6d780f6d3146a633e3755a1f
    HEAD_REF master
)

# Upstream layout is src/rectpack2D/*.h; keep the rectpack2D/ subdir so the
# headers' relative sibling includes resolve and the engine includes
# <rectpack2D/finders_interface.h>.
file(GLOB _hdrs "${SOURCE_PATH}/src/rectpack2D/*.h")
file(INSTALL ${_hdrs} DESTINATION "${CURRENT_PACKAGES_DIR}/include/rectpack2D")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/unofficial-rectpack2d/unofficial-rectpack2d-config.cmake"
"if(NOT TARGET unofficial::rectpack2d::rectpack2d)
  add_library(unofficial::rectpack2d::rectpack2d INTERFACE IMPORTED)
  set_target_properties(unofficial::rectpack2d::rectpack2d PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/../../include\")
endif()
")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
