# Per-file settings for the macOS-internal tests, mirroring the COMPILER_FLAGS
# and file types that macOS/OpenSiv3D.xcodeproj gives them upstream; keep the
# lists in sync with that project. These tests include engine internals and
# SDK headers whose global names (Point, Rect in MacTypes.h) clash with the
# 'using namespace s3d' of Siv3D.hpp, and most of them use Objective-C.
#
# set_source_files_properties is directory-scoped: call this from the
# directory that defines <target>, with the project's OBJCXX language enabled.
function(siv3d_configure_macos_test_sources target siv3d_src test_dir)
    set(_no_using
        Test_GLFW.cpp
        Test_Keyboard.cpp
        Test_MetalFrameErrors.cpp
        Test_MetalFrameSynchronization.cpp
        Test_MetalInternalTexture2D.cpp
        Test_MetalPresentation.cpp
        Test_MetalRenderPipelineState.cpp)
    set(_objcxx
        Test_GLFW.cpp
        Test_MetalFrameErrors.cpp
        Test_MetalFrameSynchronization.cpp
        Test_MetalInternalTexture2D.cpp
        Test_MetalPresentation.cpp
        Test_MetalRenderPipelineState.cpp)
    list(TRANSFORM _no_using PREPEND "${test_dir}/")
    list(TRANSFORM _objcxx PREPEND "${test_dir}/")

    set_source_files_properties(${_no_using} PROPERTIES
        COMPILE_DEFINITIONS NO_S3D_USING
        INCLUDE_DIRECTORIES "${siv3d_src};${siv3d_src}/Siv3D-Platform/macOS;${siv3d_src}/ThirdParty")
    # The Xcode Siv3D-Test target enables ARC (the engine target does not);
    # these tests rely on it, e.g. for __weak references.
    set_source_files_properties(${_objcxx} PROPERTIES
        LANGUAGE OBJCXX
        COMPILE_OPTIONS -fobjc-arc)
    # Same dialect as the C++ sources, as for Siv3DCore's .mm files.
    set_target_properties(${target} PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF)
endfunction()
