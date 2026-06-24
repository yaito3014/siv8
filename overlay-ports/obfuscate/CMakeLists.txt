cmake_minimum_required(VERSION 3.15)
project(obfuscate CXX)
include(GNUInstallDirs)

# Header-only: an INTERFACE target carrying the include dir.
add_library(obfuscate INTERFACE)
target_include_directories(obfuscate INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS obfuscate EXPORT obfuscate-targets)
install(FILES obfuscate.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# The config the engine consumes: exports unofficial::obfuscate::obfuscate.
install(EXPORT obfuscate-targets
    NAMESPACE unofficial::obfuscate::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/unofficial-obfuscate
    FILE unofficial-obfuscate-config.cmake)
