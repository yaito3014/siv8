cmake_minimum_required(VERSION 3.15)
project(xoshiro-cpp CXX)
include(GNUInstallDirs)

add_library(xoshiro-cpp INTERFACE)
target_include_directories(xoshiro-cpp INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS xoshiro-cpp EXPORT xoshiro-cpp-targets)
install(FILES XoshiroCpp.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT xoshiro-cpp-targets
    NAMESPACE unofficial::xoshiro-cpp::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/unofficial-xoshiro-cpp
    FILE unofficial-xoshiro-cpp-config.cmake)
