cmake_minimum_required(VERSION 3.12)

project(OpenSiv3D_Linux
    VERSION 0.8.0
    LANGUAGES CXX C ASM
    HOMEPAGE_URL "https://github.com/Siv3D/OpenSiv3D"
    DESCRIPTION "A C++23 framework for creative coding (2D/3D games, media art, visualizers, and simulators)."
)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SIV3D_THIRD_PARTY REQUIRED uuid)

find_package(Boost 1.71.0...1.83.0 REQUIRED)
message(STATUS "[info] Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "[info] SIV3D_THIRD_PARTY_INCLUDE_DIRS: ${SIV3D_THIRD_PARTY_INCLUDE_DIRS}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(GNUInstallDirs)

# C flags
set(SIV3D_C_FLAGS -Wall -Wextra -Wno-missing-field-initializers -fPIC -msse4.1)
set(SIV3D_C_FLAGS_DEBUG -g3 -O0 -pg -DDEBUG)
set(SIV3D_C_FLAGS_RELEASE -O2 -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_C_FLAGS_RELWITHDEBINFO -g3 -Og -pg)
set(SIV3D_C_FLAGS_MINSIZEREL -Os -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_C_DEFINITION
    _GLFW_X11
    WITH_ALSA
    WITH_NOSOUND
)

# C++ flags
set(SIV3D_CXX_FLAGS -Wall -Wextra -Wno-unknown-pragmas -fPIC -msse4.1)
set(SIV3D_CXX_FLAGS_DEBUG -g3 -O0 -pg -DDEBUG)
set(SIV3D_CXX_FLAGS_RELEASE -O2 -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_CXX_FLAGS_RELWITHDEBINFO -g3 -Og -pg)
set(SIV3D_CXX_FLAGS_MINSIZEREL -Os -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_CXX_DEFINITION
    _UNICODE
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(SIV3D_CXX_FLAGS_WITH_COMPILER -fcolor-diagnostics)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(SIV3D_CXX_FLAGS_WITH_COMPILER -fdiagnostics-color=always)
endif()

if (NOT CMAKE_CONFIGURATION_TYPES AND 
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SIV3D_INTERNAL_SOURCES
  ../Siv3D/src/Siv3D-Platform/Linux/Siv3DMain.cpp

  ../Siv3D/src/Siv3D-Platform/Linux/Time/SivTime.cpp
  
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/BinaryReader/BinaryReaderDetail.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Console/CConsole.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Date/SivDate_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/DateTime/SivDateTime_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/FileSystem/SivFileSystem_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Logger/CLogger.cpp

  ../Siv3D/src/ThirdParty/double-conversion/bignum-dtoa.cc
  ../Siv3D/src/ThirdParty/double-conversion/bignum.cc
  ../Siv3D/src/ThirdParty/double-conversion/cached-powers.cc
  ../Siv3D/src/ThirdParty/double-conversion/double-to-string.cc
  ../Siv3D/src/ThirdParty/double-conversion/fast-dtoa.cc
  ../Siv3D/src/ThirdParty/double-conversion/fixed-dtoa.cc
  ../Siv3D/src/ThirdParty/fmt/format.cc
  ../Siv3D/src/ThirdParty/simdutf/simdutf.cpp

  ../Siv3D/src/Siv3D/BigInt/SivBigInt.cpp
  ../Siv3D/src/Siv3D/BinaryReader/SivBinaryReader.cpp
  ../Siv3D/src/Siv3D/Blob/SivBlob.cpp
  ../Siv3D/src/Siv3D/BoolToString/SivBoolToString.cpp
  ../Siv3D/src/Siv3D/Byte/SivByte.cpp
  ../Siv3D/src/Siv3D/Console/ConsoleFactory.cpp
  ../Siv3D/src/Siv3D/Console/SivConsole.cpp
  ../Siv3D/src/Siv3D/ConsoleBuffer/SivConsoleBuffer.cpp
  ../Siv3D/src/Siv3D/Date/SivDate.cpp
  ../Siv3D/src/Siv3D/DateTime/SivDateTime.cpp
  ../Siv3D/src/Siv3D/DayOfWeek/SivDayOfWeek.cpp
  ../Siv3D/src/Siv3D/Demangle/SivDemangle.cpp
  ../Siv3D/src/Siv3D/Duration/SivDuration.cpp
  ../Siv3D/src/Siv3D/Engine/Siv3DEngine.cpp
  ../Siv3D/src/Siv3D/EngineLog/SivEngineLog.cpp
  ../Siv3D/src/Siv3D/Error/SivError.cpp
  ../Siv3D/src/Siv3D/EscapedFormatter/SivEscapedFormatter.cpp
  ../Siv3D/src/Siv3D/FileAction/SivFileAction.cpp
  ../Siv3D/src/Siv3D/FileSystem/SivFileSystem.cpp
  ../Siv3D/src/Siv3D/FloatFormatter/SivFloatFormatter.cpp
  ../Siv3D/src/Siv3D/FloatToString/SivFloatToString.cpp
  ../Siv3D/src/Siv3D/FmtExtension/SivFmtExtension.cpp
  ../Siv3D/src/Siv3D/FormatUtility/SivFormatUtility.cpp
  ../Siv3D/src/Siv3D/Formatter/SivFormatter.cpp
  ../Siv3D/src/Siv3D/Hash/SivHash.cpp
  ../Siv3D/src/Siv3D/Int128/SivInt128.cpp
  ../Siv3D/src/Siv3D/IntFormatter/SivIntFormatter.cpp
  ../Siv3D/src/Siv3D/IntToString/SivIntToString.cpp
  ../Siv3D/src/Siv3D/Logger/LoggerFactory.cpp
  ../Siv3D/src/Siv3D/Logger/SivLogger.cpp
  ../Siv3D/src/Siv3D/LoggerBuffer/SivLoggerBuffer.cpp
  ../Siv3D/src/Siv3D/MD5Value/SivMD5Value.cpp
  ../Siv3D/src/Siv3D/Optional/SivOptional.cpp
  ../Siv3D/src/Siv3D/ParseBool/SivParseBool.cpp
  ../Siv3D/src/Siv3D/ParseBoolExtended/SivParseBoolExtended.cpp
  ../Siv3D/src/Siv3D/ParseErrorReason/SivParseErrorReason.cpp
  ../Siv3D/src/Siv3D/ParseFloat/SivParseFloat.cpp
  ../Siv3D/src/Siv3D/ParseInt/SivParseInt.cpp
  ../Siv3D/src/Siv3D/Point/SivPoint.cpp
  ../Siv3D/src/Siv3D/Random/SivRandom.cpp
  ../Siv3D/src/Siv3D/Rect/SivRect.cpp
  ../Siv3D/src/Siv3D/String/Levenshtein.cpp
  ../Siv3D/src/Siv3D/String/SivString.cpp
  ../Siv3D/src/Siv3D/StringView/SivStringView.cpp
  ../Siv3D/src/Siv3D/System/SivSystem.cpp
  ../Siv3D/src/Siv3D/TextEncoding/SivTextEncoding.cpp
  ../Siv3D/src/Siv3D/TextReader/SivTextReader.cpp
  ../Siv3D/src/Siv3D/TextReader/TextReaderDetail.cpp
  ../Siv3D/src/Siv3D/ThousandSeparate/SivThousandSeparate.cpp
  ../Siv3D/src/Siv3D/Threading/SivThreading.cpp
  ../Siv3D/src/Siv3D/UUIDValue/SivUUIDValue.cpp
  ../Siv3D/src/Siv3D/Unicode/SivUnicode.cpp
  ../Siv3D/src/Siv3D/UnicodeConverter/SivUnicodeConverter.cpp
  ../Siv3D/src/Siv3D/UnicodeConverter/UnicodeUtility.cpp
  ../Siv3D/src/Siv3D/Vector2D/SivVector2D.cpp
  ../Siv3D/src/Siv3D/_TEMPLATE/_TEMPLATE.cpp
)

set(SIV3D_INTERNAL_INCLUDE_DIRS
  ../Siv3D/include
  ../Siv3D/include/ThirdParty
  ../Siv3D/src
  ../Siv3D/src/Siv3D-Platform/Linux
  ../Siv3D/src/Siv3D-Platform/macOS_Linux
  ../Siv3D/src/ThirdParty
  ${SIV3D_THIRD_PARTY_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)


add_library(Siv3D STATIC ${SIV3D_INTERNAL_SOURCES})
target_link_libraries(Siv3D INTERFACE
  ${CMAKE_THREAD_LIBS_INIT}
  ${SIV3D_THIRD_PARTY_LIBRARIES}
  ${CMAKE_DL_LIBS}
)
target_compile_features(Siv3D PRIVATE cxx_std_23 c_std_11)
target_include_directories(Siv3D 
  PRIVATE
    $<BUILD_INTERFACE: ${SIV3D_INTERNAL_INCLUDE_DIRS}>
    $<BUILD_INTERFACE: ${SIV3D_THIRD_PARTY_INCLUDE_DIRS}>
  INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D/ThirdParty>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Siv3D>
)
set_target_properties(Siv3D PROPERTIES
  C_EXTENSIONS OFF
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  DEBUG_POSTFIX "d"
)
target_compile_options(Siv3D PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>: ${SIV3D_CXX_FLAGS} ${SIV3D_CXX_FLAGS_WITH_COMPILER}
    $<$<CONFIG:Debug>:   ${SIV3D_CXX_FLAGS_DEBUG}>
    $<$<CONFIG:Release>: ${SIV3D_CXX_FLAGS_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>: ${SIV3D_CXX_FLAGS_RELWITHDEBINFO}>
    $<$<CONFIG:MinSizeRel>: ${SIV3D_CXX_FLAGS_MINSIZEREL}>
  >
  $<$<COMPILE_LANGUAGE:C>: ${SIV3D_C_FLAGS} ${SIV3D_CXX_FLAGS_WITH_COMPILER}
    $<$<CONFIG:Debug>:   ${SIV3D_C_FLAGS_DEBUG}>
    $<$<CONFIG:Release>: ${SIV3D_C_FLAGS_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>: ${SIV3D_C_FLAGS_RELWITHDEBINFO}>
    $<$<CONFIG:MinSizeRel>: ${SIV3D_C_FLAGS_MINSIZEREL}>
  >
)
target_compile_definitions(Siv3D PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>: ${SIV3D_CXX_DEFINITION}>
  $<$<COMPILE_LANGUAGE:C>:   ${SIV3D_C_DEFINITION}>
)
add_library(Siv3D::Siv3D ALIAS Siv3D)


# Siv3D install settings
message(STATUS "[info] CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message(STATUS "[info] CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/Siv3D")
install(
  TARGETS Siv3D
  EXPORT Siv3DTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
  EXPORT Siv3DTargets
  FILE Siv3DConfig.cmake
  NAMESPACE Siv3D::
  EXPORT_LINK_INTERFACE_LIBRARIES
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Siv3D
)
install(
  FILES ../Siv3D/include/Siv3D.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Siv3D
)
install(
  DIRECTORY ../Siv3D/include/Siv3D ../Siv3D/include/ThirdParty
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Siv3D
)

# deb package setttings
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT ${CMAKE_PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_HOMEPAGE_URL ${CMAKE_PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_NAME "libSiv3D-dev")
set(CPACK_PACKAGE_FILE_LICENSE ../LICENSE)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "cmake, libboost-dev, uuid-dev")

include(CPack)
