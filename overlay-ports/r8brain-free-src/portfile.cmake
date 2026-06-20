vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO avaneev/r8brain-free-src
    REF e71c31bf320f84210bb4bdcb57e296c39ce940f9
    SHA512 77a6f4277f65d68fd14736a8da533ec0f1bed19af0c0ce132c1c0af4d21d321cd81ec23cef3b5b153d7e8221fca7b4f157c6307da3a29dbf594562bf0374ebbf
    HEAD_REF master
)

# Siv3D: use the separate pffft overlay (unevens) for the FFT rather than the
# bundled fft/pffft copy — matching how Siv3D vendored it.
vcpkg_replace_string("${SOURCE_PATH}/CDSPRealFFT.h"
    "#include \"fft/pffft_double.h\"" "#include <pffft/pffft_double.h>")
vcpkg_replace_string("${SOURCE_PATH}/CDSPRealFFT.h"
    "#include \"fft/pffft.h\"" "#include <pffft/pffft.h>")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-r8brain-free-src CONFIG_PATH share/unofficial-r8brain-free-src)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
