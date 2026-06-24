cmake_minimum_required(VERSION 3.15)
project(enumbitmask CXX)
include(GNUInstallDirs)

add_library(enumbitmask INTERFACE)
target_include_directories(enumbitmask INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS enumbitmask EXPORT enumbitmask-targets)
install(FILES EnumBitmask.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT enumbitmask-targets
    NAMESPACE unofficial::enumbitmask::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/unofficial-enumbitmask
    FILE unofficial-enumbitmask-config.cmake)
