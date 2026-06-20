cmake_minimum_required(VERSION 3.15)
project(easyexif CXX)
include(GNUInstallDirs)

add_library(easyexif STATIC exif.cpp)
target_include_directories(easyexif PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
set_target_properties(easyexif PROPERTIES CXX_STANDARD 11 PUBLIC_HEADER exif.h)

install(TARGETS easyexif EXPORT easyexif-targets
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# The config the registry port lacks: exports unofficial::easyexif::easyexif.
install(EXPORT easyexif-targets
    NAMESPACE unofficial::easyexif::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/unofficial-easyexif
    FILE unofficial-easyexif-config.cmake)
