cmake_minimum_required(VERSION 3.15)
project(r8brain-free-src CXX)
include(GNUInstallDirs)

find_package(pffft CONFIG REQUIRED)

# Header-only: an INTERFACE target carrying the include dir + the pffft dep
# (CDSPRealFFT.h is repointed to <pffft/...> in the portfile).
add_library(r8brain-free-src INTERFACE)
target_include_directories(r8brain-free-src INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/r8brain-free-src>)
target_link_libraries(r8brain-free-src INTERFACE pffft::pffft)

install(TARGETS r8brain-free-src EXPORT r8brain-targets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/r8brain-free-src"
    FILES_MATCHING PATTERN "*.h" PATTERN "*.inc")

install(EXPORT r8brain-targets
    NAMESPACE unofficial::r8brain-free-src::
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/unofficial-r8brain-free-src"
    FILE unofficial-r8brain-free-src-config.cmake)
