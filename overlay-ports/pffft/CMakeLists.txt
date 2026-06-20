cmake_minimum_required(VERSION 3.12.4)
project(pffft C)

# marton78/pffft layout: sources in src/, public headers in include/pffft/, SIMD
# impls in src/simd/. The .c files include "pffft.h"/"pffft_double.h" (from
# include/pffft) and "simd/..."/"pffft_priv_impl.h" (relative, from src/).
add_library(pffft src/pffft.c src/pffft_common.c src/pffft_double.c)

if(MSVC)
    target_compile_definitions(pffft PRIVATE _USE_MATH_DEFINES)
endif()

target_include_directories(pffft
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/pffft"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")

install(TARGETS pffft EXPORT pffft-config)
install(EXPORT pffft-config NAMESPACE pffft:: DESTINATION share/pffft)

if(NOT DISABLE_INSTALL_HEADERS)
    install(FILES include/pffft/pffft.h include/pffft/pffft_double.h DESTINATION include/pffft)
endif()
