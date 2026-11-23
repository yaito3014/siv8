# Prebuilt PDFium from bblanchon/pdfium-binaries. Siv3D loads the shared library
# at run time and compiles only against the headers, so the port installs the
# headers, the shared library and a CMake config, but no import library.
#
# To update: change PDFIUM_RELEASE (and the version in vcpkg.json), download the
# six archives with
#   gh release download <release> -R bblanchon/pdfium-binaries -p 'pdfium-*-*64.tgz'
# check them with `gh attestation verify <archive> --repo bblanchon/pdfium-binaries`,
# and replace the SHA512 values below with `sha512sum`.
set(PDFIUM_RELEASE "chromium/7725")
set(PDFIUM_SHA512_win-x64     214b2ac6b21ee6f5b716cf6283068091563cdd4a4accdefa0752460ad10e516590a635fa303ed8da0e3707bc35ec195ec3ca9ce9e25407e790e121e44338f839)
set(PDFIUM_SHA512_win-arm64   3a9c5c1f92b03027fecb376513e68ba1f9cd0ae5b1180485bb5aa74d6bc136cf21c1d828833cedef05618cc6b235d5d4651d238966d10a6099e0bc5908da9be9)
set(PDFIUM_SHA512_mac-x64     d2eff95ab882f3637c173b6a8b8607adcf7dfa9b75edb90009175827a465c466a7dedae33ac5d2e10737be1b8f3f26b64ba2d98c6176f3fe41ad7e3132aa4b2c)
set(PDFIUM_SHA512_mac-arm64   bc6ddee9782c36ceedd08477afb892e67026f1fb609e2d134a55e031f51d851c29963a298214fe5ac71d28ef8bbbf590a8432350d94308ff745c01fcea8baadf)
set(PDFIUM_SHA512_linux-x64   63f7791253333b25b41330fce7d09873374db8959d30699cdb50f78927a7ad91155d139b2db44c827b86710c5ebbb838c2383ae3f6be3be8bc30fb59faffffde)
set(PDFIUM_SHA512_linux-arm64 c7526f4558e0e302132ac2c2c2239541b486d27220418e3168968ebc6b4ffe3728b7769ac627d639953b534adfcbea4132d93b9dd6dc02b131d58f14372048cb)

if(VCPKG_TARGET_IS_WINDOWS)
    set(_os win)
    set(_library bin/pdfium.dll)
elseif(VCPKG_TARGET_IS_OSX)
    set(_os mac)
    set(_library lib/libpdfium.dylib)
else()
    set(_os linux)
    set(_library lib/libpdfium.so)
endif()
set(_platform "${_os}-${VCPKG_TARGET_ARCHITECTURE}")
set(_archive "pdfium-${_platform}")
string(REPLACE "/" "%2F" _release_url "${PDFIUM_RELEASE}")
string(REPLACE "/" "-" _release_file "${PDFIUM_RELEASE}")

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/bblanchon/pdfium-binaries/releases/download/${_release_url}/${_archive}.tgz"
    FILENAME "pdfium-binaries-${_release_file}-${_archive}.tgz"
    SHA512 "${PDFIUM_SHA512_${_platform}}"
)
vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE "${ARCHIVE}" NO_REMOVE_ONE_LEVEL)

# Under include/pdfium so the C headers do not spread over the shared include root.
file(GLOB _headers "${SOURCE_PATH}/include/*.h")
file(INSTALL ${_headers} "${SOURCE_PATH}/include/cpp" DESTINATION "${CURRENT_PACKAGES_DIR}/include/pdfium")

# There is no debug build: the debug tree gets the same library, which is fine
# for a C API loaded at run time, independent of the consumer's C runtime.
get_filename_component(_library_dir "${_library}" DIRECTORY)
file(INSTALL "${SOURCE_PATH}/${_library}" DESTINATION "${CURRENT_PACKAGES_DIR}/${_library_dir}")
file(INSTALL "${SOURCE_PATH}/${_library}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/${_library_dir}")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/unofficial-pdfium-binaries-config.cmake"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/unofficial-pdfium-binaries")

file(GLOB _licenses "${SOURCE_PATH}/licenses/*")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE" ${_licenses})

# A shared library in static triplets, without an import library, whose C
# runtime is its own.
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_SKIP_CRT_LINKAGE_CHECK enabled)
