# Script mode (cmake -P): fails if the fat lib's input list names an archive
# outside the vcpkg triplet directory. System libraries belong in the link
# interface of Siv3D::Siv3D, not inside the archive.
#   -DSIV3D_INPUTS_FILE=<one archive per line, optionally quoted>
#   -DSIV3D_VCPKG_TRIPLET_DIR=<vcpkg_installed>/<triplet>
cmake_minimum_required(VERSION 3.25)

file(STRINGS "${SIV3D_INPUTS_FILE}" _lines)
string(TOLOWER "${SIV3D_VCPKG_TRIPLET_DIR}/" _prefix)
set(_outside "")
foreach(_line IN LISTS _lines)
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" _path "${_line}")
    string(TOLOWER "${_path}" _lower)
    string(FIND "${_lower}" "${_prefix}" _at)
    if(NOT _at EQUAL 0)
        list(APPEND _outside "${_path}")
    endif()
endforeach()
if(_outside)
    list(JOIN _outside "\n  " _outside)
    message(FATAL_ERROR "The fat lib bundles archives from outside ${SIV3D_VCPKG_TRIPLET_DIR}:\n  ${_outside}")
endif()
