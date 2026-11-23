# unofficial::pdfium-binaries::headers  the PDFium C headers
# unofficial::pdfium-binaries::library  the shared library, as a MODULE target:
#   it is meant to be loaded at run time and cannot be linked; use
#   $<TARGET_FILE:unofficial::pdfium-binaries::library> to stage it.
# unofficial-pdfium-binaries_LICENSE    the license texts to ship with it.
get_filename_component(_pdfium_prefix "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(unofficial-pdfium-binaries_LICENSE "${_pdfium_prefix}/share/pdfium-binaries/copyright")

if(NOT TARGET unofficial::pdfium-binaries::headers)
    add_library(unofficial::pdfium-binaries::headers INTERFACE IMPORTED)
    set_target_properties(unofficial::pdfium-binaries::headers PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_pdfium_prefix}/include/pdfium")
endif()

if(NOT TARGET unofficial::pdfium-binaries::library)
    if(WIN32)
        set(_pdfium_library "${_pdfium_prefix}/bin/pdfium.dll")
    elseif(APPLE)
        set(_pdfium_library "${_pdfium_prefix}/lib/libpdfium.dylib")
    else()
        set(_pdfium_library "${_pdfium_prefix}/lib/libpdfium.so")
    endif()
    add_library(unofficial::pdfium-binaries::library MODULE IMPORTED)
    set_target_properties(unofficial::pdfium-binaries::library PROPERTIES
        IMPORTED_LOCATION "${_pdfium_library}")
    unset(_pdfium_library)
endif()

unset(_pdfium_prefix)
