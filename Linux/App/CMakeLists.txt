cmake_minimum_required(VERSION 3.12)
project(OpenSiv3D_Linux_App CXX)

if (NOT CMAKE_CONFIGURATION_TYPES AND 
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(Siv3DTest
Main.cpp
../../Test/Siv3DTest.cpp
../../Test/Test_Array.cpp
../../Test/Test_Base64Value.cpp
../../Test/Test_BinaryWriter.cpp
../../Test/Test_Byte.cpp
../../Test/Test_Color.cpp
../../Test/Test_ColorF.cpp
../../Test/Test_Compression.cpp
../../Test/Test_Concepts.cpp
#../../Test/Test_FileSystem.cpp
../../Test/Test_FmtExtension.cpp
../../Test/Test_FunctionRef.cpp
../../Test/Test_Hash.cpp
../../Test/Test_Image.cpp
../../Test/Test_Indexed.cpp
../../Test/Test_JSON.cpp
../../Test/Test_Memory.cpp
../../Test/Test_MemoryMappedFile.cpp
../../Test/Test_MemoryMappedFileView.cpp
../../Test/Test_NamedParameter.cpp
../../Test/Test_PRNG.cpp
../../Test/Test_Platform.cpp
../../Test/Test_ScopeExit.cpp
../../Test/Test_Step.cpp
../../Test/Test_String.cpp
../../Test/Test_StringView.cpp
../../Test/Test_TextReader.cpp
../../Test/Test_TupleFormatter.cpp
../../Test/Test_Types.cpp
../../Test/Test_Unicode.cpp
../../Test/Test_Utility.cpp
../../Test/Test_Version.cpp
../../Test/Test_YesNo.cpp
)

find_package(Siv3D)
target_link_libraries(Siv3DTest PUBLIC Siv3D::Siv3D)

target_compile_features(Siv3DTest PRIVATE cxx_std_23)
target_compile_options(Siv3DTest PRIVATE -msse4.2)

if(BUILD_TESTING)
enable_testing()
add_test(
  NAME Test
  COMMAND Siv3DTest
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endif()
