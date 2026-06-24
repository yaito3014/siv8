cmake_minimum_required(VERSION 3.15)
project(libcpprime CXX)
include(GNUInstallDirs)

add_library(libcpprime INTERFACE)
target_include_directories(libcpprime INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS libcpprime EXPORT libcpprime-targets)
# The whole libcpprime/ tree: IsPrimeNoTable.hpp + internal/ (headers + .txt tables).
install(DIRECTORY include/libcpprime DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libcpprime-targets
    NAMESPACE unofficial::libcpprime::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/unofficial-libcpprime
    FILE unofficial-libcpprime-config.cmake)
