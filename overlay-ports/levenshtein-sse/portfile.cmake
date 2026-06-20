# Header-only. Provenance: pinned upstream commit; the only local change is the
# reviewable patch below (replaces what was previously a bare-vendored, silently
# modified copy). A real fork would set REPO to the fork instead.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO addaleax/levenshtein-sse
    REF b67bd51fe3be467d7036d722ea57a6587f005fe6
    SHA512 ed6cb094e9680808c63047c539b979591adcf7bba1ce6b02cdb348b95353d74613b6469202df7eafe73cb345dc35c7ad15a9d579221c5216fec2103eaa982532
    HEAD_REF master
    PATCHES
        0001-siv3d-changes.patch
)

file(INSTALL "${SOURCE_PATH}/levenshtein-sse.hpp"
     DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# Header-only INTERFACE target. The config dir must match the find_package name
# (share/unofficial-levenshtein-sse/), not the port name.
file(WRITE "${CURRENT_PACKAGES_DIR}/share/unofficial-levenshtein-sse/unofficial-levenshtein-sse-config.cmake"
"if(NOT TARGET unofficial::levenshtein-sse::levenshtein-sse)
  add_library(unofficial::levenshtein-sse::levenshtein-sse INTERFACE IMPORTED)
  set_target_properties(unofficial::levenshtein-sse::levenshtein-sse PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES \"\${CMAKE_CURRENT_LIST_DIR}/../../include\")
endif()
")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
