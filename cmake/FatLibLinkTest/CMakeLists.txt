cmake_minimum_required(VERSION 3.25)
project(Siv3DFatLibLinkTest LANGUAGES CXX)

# Links every member of the installed fat lib, so each external any member
# references must resolve: a system library missing from Siv3D::Siv3D's
# interface, or a definition missing from the archive, fails this link.
find_package(Siv3D CONFIG REQUIRED)

add_executable(Siv3DFatLibLinkTest WIN32 Main.cpp)
target_link_libraries(Siv3DFatLibLinkTest PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,Siv3D::Siv3D>")
if(MSVC)
    set_property(TARGET Siv3DFatLibLinkTest PROPERTY MSVC_RUNTIME_LIBRARY "${Siv3D_MSVC_RUNTIME_LIBRARY}")
    # The archive holds FreeType's version resource; the linker's embedded
    # manifest would be a second resource file (LNK1241).
    target_link_options(Siv3DFatLibLinkTest PRIVATE /MANIFEST:NO)
endif()
