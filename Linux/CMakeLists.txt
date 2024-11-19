cmake_minimum_required(VERSION 3.12)

project(OpenSiv3D_Linux
    VERSION 0.8
    LANGUAGES CXX C ASM
    HOMEPAGE_URL "https://github.com/Siv3D/OpenSiv3D"
    DESCRIPTION "A C++23 framework for creative coding (2D/3D games, media art, visualizers, and simulators)."
)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(SIV3D_THIRD_PARTY REQUIRED freetype2 gio-2.0 gl glu glib-2.0 harfbuzz libpng libturbojpeg uuid x11 zlib)

find_package(Boost 1.83.0 CONFIG REQUIRED)
message(STATUS "[info] Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "[info] SIV3D_THIRD_PARTY_INCLUDE_DIRS: ${SIV3D_THIRD_PARTY_INCLUDE_DIRS}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(GNUInstallDirs)

# C flags
set(SIV3D_C_FLAGS -Wall -Wextra -Wno-missing-field-initializers -fPIC -msse4.2)
set(SIV3D_C_FLAGS_DEBUG -g3 -O0 -pg -DDEBUG)
set(SIV3D_C_FLAGS_RELEASE -O2 -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_C_FLAGS_RELWITHDEBINFO -g3 -Og -pg)
set(SIV3D_C_FLAGS_MINSIZEREL -Os -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_C_DEFINITION
    _GLFW_X11
)

# C++ flags
set(SIV3D_CXX_FLAGS -Wall -Wextra -Wno-unknown-pragmas -fPIC -msse4.2 -mavx2 -mf16c -mbmi2)
set(SIV3D_CXX_FLAGS_DEBUG -g3 -O0 -pg -DDEBUG)
set(SIV3D_CXX_FLAGS_RELEASE -O2 -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_CXX_FLAGS_RELWITHDEBINFO -g3 -Og -pg)
set(SIV3D_CXX_FLAGS_MINSIZEREL -Os -DNDEBUG -march=x86-64 -mtune=generic)
set(SIV3D_CXX_DEFINITION
    _GLFW_X11
    MUPARSER_STATIC
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
  ../Siv3D/src/Siv3D-Platform/Linux/Siv3D/Cursor/CCursor.cpp
  ../Siv3D/src/Siv3D-Platform/Linux/Siv3D/FileSystem/SivFileSystem_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/Linux/Siv3D/Time/SivTime.cpp

  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/BinaryReader/BinaryReaderDetail.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/BinaryWriter/BinaryWriterDetail.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Console/CConsole.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/CursorStyle/CCursorStyle.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Date/SivDate_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/DateTime/SivDateTime_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/EnvironmentVariable/SivEnvironmentVariable.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/FileSystem/SivFileSystem_macOS_Linux.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Keyboard/CKeyboard.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Logger/CLogger.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/MemoryMappedFile/MemoryMappedFileDetail.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/MemoryMappedFileView/MemoryMappedFileViewDetail.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/Mouse/CMouse.cpp
  ../Siv3D/src/Siv3D-Platform/macOS_Linux/Siv3D/System/SivSystem_macOS_Linux.cpp

  ../Siv3D/src/Siv3D/Array/SivArray.cpp
  ../Siv3D/src/Siv3D/Base64Value/SivBase64Value.cpp
  ../Siv3D/src/Siv3D/BigFloat/SivBigFloat.cpp
  ../Siv3D/src/Siv3D/BigInt/SivBigInt.cpp
  ../Siv3D/src/Siv3D/BigNumMath/SivBigNumMath.cpp
  ../Siv3D/src/Siv3D/BinaryReader/SivBinaryReader.cpp
  ../Siv3D/src/Siv3D/BinaryWriter/SivBinaryWriter.cpp
  ../Siv3D/src/Siv3D/BlendFactor/SivBlendFactor.cpp
  ../Siv3D/src/Siv3D/BlendOperation/SivBlendOperation.cpp
  ../Siv3D/src/Siv3D/BlendState/SivBlendState.cpp
  ../Siv3D/src/Siv3D/BlendStateBuilder/SivBlendStateBuilder.cpp
  ../Siv3D/src/Siv3D/Blob/SivBlob.cpp
  ../Siv3D/src/Siv3D/BoolToString/SivBoolToString.cpp
  ../Siv3D/src/Siv3D/Byte/SivByte.cpp
  ../Siv3D/src/Siv3D/CPUInfo/SivCPUInfo.cpp
  ../Siv3D/src/Siv3D/CacheDirectory/CacheDirectory.cpp
  ../Siv3D/src/Siv3D/Camera2D/SivCamera2D.cpp
  ../Siv3D/src/Siv3D/Camera2DControl/SivCamera2DControl.cpp
  ../Siv3D/src/Siv3D/Camera2DControlBuilder/SivCamera2DControlBuilder.cpp
  ../Siv3D/src/Siv3D/Circle/SivCircle.cpp
  ../Siv3D/src/Siv3D/Color/SivColor.cpp
  ../Siv3D/src/Siv3D/ColorF/SivColorF.cpp
  ../Siv3D/src/Siv3D/ColorWriteMask/SivColorWriteMask.cpp
  ../Siv3D/src/Siv3D/Colormap/SivColormap.cpp
  ../Siv3D/src/Siv3D/CommandLine/SivCommandLine.cpp
  ../Siv3D/src/Siv3D/CompareFunction/SivCompareFunction.cpp
  ../Siv3D/src/Siv3D/Compression/SivCompression.cpp
  ../Siv3D/src/Siv3D/Console/ConsoleFactory.cpp
  ../Siv3D/src/Siv3D/Console/SivConsole.cpp
  ../Siv3D/src/Siv3D/ConsoleBuffer/SivConsoleBuffer.cpp
  ../Siv3D/src/Siv3D/CullMode/SivCullMode.cpp
  ../Siv3D/src/Siv3D/Cursor/CursorFactory.cpp
  ../Siv3D/src/Siv3D/Cursor/CursorTransform.cpp
  ../Siv3D/src/Siv3D/Cursor/HighTemporalResolutionCursor.cpp
  ../Siv3D/src/Siv3D/Cursor/SivCursor.cpp
  ../Siv3D/src/Siv3D/CursorStyle/CursorStyleFactory.cpp
  ../Siv3D/src/Siv3D/CursorStyle/SivCursorStyle.cpp
  ../Siv3D/src/Siv3D/CycleClock/SivCycleClock.cpp
  ../Siv3D/src/Siv3D/Date/SivDate.cpp
  ../Siv3D/src/Siv3D/DateTime/SivDateTime.cpp
  ../Siv3D/src/Siv3D/DayOfWeek/SivDayOfWeek.cpp
  ../Siv3D/src/Siv3D/Demangle/SivDemangle.cpp
  ../Siv3D/src/Siv3D/DepthStencilState/SivDepthStencilState.cpp
  ../Siv3D/src/Siv3D/DepthStencilStateBuilder/SivDepthStencilStateBuilder.cpp
  # ../Siv3D/src/Siv3D/DirectoryWatcher/SivDirectoryWatcher.cpp
  ../Siv3D/src/Siv3D/Duration/SivDuration.cpp
  ../Siv3D/src/Siv3D/Emoji/CEmoji.cpp
  ../Siv3D/src/Siv3D/Emoji/EmojiFactory.cpp
  ../Siv3D/src/Siv3D/Emoji/SivEmoji.cpp
  ../Siv3D/src/Siv3D/Empty/CEmpty.cpp
  ../Siv3D/src/Siv3D/Empty/EmptyFactory.cpp
  ../Siv3D/src/Siv3D/Engine/Siv3DEngine.cpp
  ../Siv3D/src/Siv3D/EngineLog/SivEngineLog.cpp
  ../Siv3D/src/Siv3D/EngineOption/SivEngineOption.cpp
  ../Siv3D/src/Siv3D/EngineResource/CEngineResource.cpp
  ../Siv3D/src/Siv3D/EngineResource/EngineResourceFactory.cpp
  ../Siv3D/src/Siv3D/Error/SivError.cpp
  ../Siv3D/src/Siv3D/EscapedFormatter/SivEscapedFormatter.cpp
  ../Siv3D/src/Siv3D/Exif/SivExif.cpp
  ../Siv3D/src/Siv3D/ExtensionFilter/ExtensionFilter.cpp
  ../Siv3D/src/Siv3D/FileAction/SivFileAction.cpp
  ../Siv3D/src/Siv3D/FileSystem/SivFileSystem.cpp
  ../Siv3D/src/Siv3D/FloatFormatter/SivFloatFormatter.cpp
  ../Siv3D/src/Siv3D/FloatToString/SivFloatToString.cpp
  ../Siv3D/src/Siv3D/FmtExtension/SivFmtExtension.cpp
  ../Siv3D/src/Siv3D/FormatUtility/SivFormatUtility.cpp
  ../Siv3D/src/Siv3D/Formatter/SivFormatter.cpp
  ../Siv3D/src/Siv3D/Geometry2D/SivGeometry2D_Intersect.cpp
  ../Siv3D/src/Siv3D/Graphics/SivGraphics.cpp
  ../Siv3D/src/Siv3D/Graphics2D/SivGraphics2D.cpp
  ../Siv3D/src/Siv3D/Grid/SivGrid.cpp
  ../Siv3D/src/Siv3D/HLSL/SivHLSL.cpp
  ../Siv3D/src/Siv3D/HSV/SivHSV.cpp
  ../Siv3D/src/Siv3D/HalfFloat/SivHalfFloat.cpp
  ../Siv3D/src/Siv3D/Hash/SivHash.cpp
  ../Siv3D/src/Siv3D/IImageDecoder/SivIImageDecoder.cpp
  ../Siv3D/src/Siv3D/INI/SivINI.cpp
  ../Siv3D/src/Siv3D/INIItem/SivINIItem.cpp
  ../Siv3D/src/Siv3D/INISection/SivINISection.cpp
  ../Siv3D/src/Siv3D/Image/SivImage.cpp
  ../Siv3D/src/Siv3D/Image/SivImage_SIMD.cpp
  ../Siv3D/src/Siv3D/ImageDecoder/CImageDecoder.cpp
  ../Siv3D/src/Siv3D/ImageDecoder/ImageDecoderFactory.cpp
  ../Siv3D/src/Siv3D/ImageDecoder/SivImageDecoder.cpp
  ../Siv3D/src/Siv3D/ImageEncoder/CImageEncoder.cpp
  ../Siv3D/src/Siv3D/ImageEncoder/ImageEncoderFactory.cpp
  ../Siv3D/src/Siv3D/ImageEncoder/SivImageEncoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/BMP/BMPDecoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/BMP/BMPEncoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/JPEG/JPEGDecoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/JPEG/JPEGEncoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/PNG/PNGDecoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/PNG/PNGEncoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/SVG/SVGDecoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/SivImageFormat.cpp
  ../Siv3D/src/Siv3D/ImageFormat/TGA/TGADecoder.cpp
  ../Siv3D/src/Siv3D/ImageFormat/TGA/TGAEncoder.cpp
  ../Siv3D/src/Siv3D/ImagePixelFormat/SivImagePixelFormat.cpp
  ../Siv3D/src/Siv3D/Input/SivInput.cpp
  ../Siv3D/src/Siv3D/InputCombination/SivInputCombination.cpp
  ../Siv3D/src/Siv3D/InputDeviceType/SivInputDeviceType.cpp
  ../Siv3D/src/Siv3D/InputGroup/SivInputGroup.cpp
  ../Siv3D/src/Siv3D/InputState/SivInputState.cpp
  ../Siv3D/src/Siv3D/Int128/SivInt128.cpp
  ../Siv3D/src/Siv3D/IntFormatter/SivIntFormatter.cpp
  ../Siv3D/src/Siv3D/IntToString/SivIntToString.cpp
  ../Siv3D/src/Siv3D/Interpolation/SivInterpolation.cpp
  ../Siv3D/src/Siv3D/JSON/SivJSON.cpp
  ../Siv3D/src/Siv3D/JSONIterator/SivJSONIterator.cpp
  ../Siv3D/src/Siv3D/JSONValueType/SivJSONValueType.cpp
  ../Siv3D/src/Siv3D/Keyboard/KeyboardFactory.cpp
  ../Siv3D/src/Siv3D/Keyboard/SivKeyboard.cpp
  ../Siv3D/src/Siv3D/LicenseManager/CLicenseManager.cpp
  ../Siv3D/src/Siv3D/LicenseManager/LicenseManagerFactory.cpp
  ../Siv3D/src/Siv3D/LicenseManager/SivLicenseManager.cpp
  ../Siv3D/src/Siv3D/Line/SivLine.cpp
  ../Siv3D/src/Siv3D/LineCap/SivLineCap.cpp
  ../Siv3D/src/Siv3D/LineType/SivLineType.cpp
  ../Siv3D/src/Siv3D/Logger/LoggerFactory.cpp
  ../Siv3D/src/Siv3D/Logger/SivLogger.cpp
  ../Siv3D/src/Siv3D/LoggerBuffer/SivLoggerBuffer.cpp
  ../Siv3D/src/Siv3D/MD5Value/SivMD5Value.cpp
  ../Siv3D/src/Siv3D/MSAASampleCount/SivMSAASampleCount.cpp
  ../Siv3D/src/Siv3D/MSL/SivMSL.cpp
  ../Siv3D/src/Siv3D/Mat3x2/SivMat3x2.cpp
  ../Siv3D/src/Siv3D/MathParser/MathParserDetail.cpp
  ../Siv3D/src/Siv3D/MathParser/SivMathParser.cpp
  ../Siv3D/src/Siv3D/MemoryMappedFile/SivMemoryMappedFile.cpp
  ../Siv3D/src/Siv3D/MemoryMappedFileView/SivMemoryMappedFileView.cpp
  ../Siv3D/src/Siv3D/MemoryReader/SivMemoryReader.cpp
  ../Siv3D/src/Siv3D/MemoryViewReader/SivMemoryViewReader.cpp
  ../Siv3D/src/Siv3D/MessageBox/SivMessageBox.cpp
  ../Siv3D/src/Siv3D/MicrosecClock/SivMicrosecClock.cpp
  ../Siv3D/src/Siv3D/MillisecClock/SivMillisecClock.cpp
  ../Siv3D/src/Siv3D/MonitorInfo/SivMonitorInfo.cpp
  ../Siv3D/src/Siv3D/Morton/SivMorton.cpp
  ../Siv3D/src/Siv3D/Mouse/MouseFactory.cpp
  ../Siv3D/src/Siv3D/Mouse/SivMouse.cpp
  ../Siv3D/src/Siv3D/Optional/SivOptional.cpp
  ../Siv3D/src/Siv3D/Parse/SivParse.cpp
  ../Siv3D/src/Siv3D/ParseBool/SivParseBool.cpp
  ../Siv3D/src/Siv3D/ParseBoolExtended/SivParseBoolExtended.cpp
  ../Siv3D/src/Siv3D/ParseErrorReason/SivParseErrorReason.cpp
  ../Siv3D/src/Siv3D/ParseFloat/SivParseFloat.cpp
  ../Siv3D/src/Siv3D/ParseInt/SivParseInt.cpp
  ../Siv3D/src/Siv3D/PixelShader/SivPixelShader.cpp
  ../Siv3D/src/Siv3D/Point/SivPoint.cpp
  ../Siv3D/src/Siv3D/Point3D/SivPoint3D.cpp
  ../Siv3D/src/Siv3D/Profiler/CProfiler.cpp
  ../Siv3D/src/Siv3D/Profiler/ProfilerFactory.cpp
  ../Siv3D/src/Siv3D/Profiler/SivProfiler.cpp
  ../Siv3D/src/Siv3D/Quad/SivQuad.cpp
  ../Siv3D/src/Siv3D/Random/SivRandom.cpp
  ../Siv3D/src/Siv3D/RasterizerState/SivRasterizerState.cpp
  ../Siv3D/src/Siv3D/RasterizerStateBuilder/SivRasterizerStateBuilder.cpp
  ../Siv3D/src/Siv3D/Rect/SivRect.cpp
  ../Siv3D/src/Siv3D/RectF/SivRectF.cpp
  ../Siv3D/src/Siv3D/RegExp/CRegExp.cpp
  ../Siv3D/src/Siv3D/RegExp/RegExpDetail.cpp
  ../Siv3D/src/Siv3D/RegExp/RegExpFactory.cpp
  ../Siv3D/src/Siv3D/RegExp/SivRegExp.cpp
  ../Siv3D/src/Siv3D/Renderer2D/Vertex2DBuilder_Rounded.cpp
  ../Siv3D/src/Siv3D/Renderer2D/Vertex2DBuilder_StraightEdged.cpp
  ../Siv3D/src/Siv3D/ResizeMode/SivResizeMode.cpp
  ../Siv3D/src/Siv3D/SIMDMath/SivSIMDMath.cpp
  ../Siv3D/src/Siv3D/SVG/SVGDetail.cpp
  ../Siv3D/src/Siv3D/SVG/SivSVG.cpp
  ../Siv3D/src/Siv3D/SamplerState/SivSamplerState.cpp
  ../Siv3D/src/Siv3D/SamplerStateBuilder/SivSamplerStateBuilder.cpp
  ../Siv3D/src/Siv3D/Scene/CScene.cpp
  ../Siv3D/src/Siv3D/Scene/SceneFactory.cpp
  ../Siv3D/src/Siv3D/Scene/SceneTime.cpp
  ../Siv3D/src/Siv3D/Scene/SceneUtility.cpp
  ../Siv3D/src/Siv3D/Scene/SivScene.cpp
  ../Siv3D/src/Siv3D/ScopedColorAdd2D/SivScopedColorAdd2D.cpp
  ../Siv3D/src/Siv3D/ScopedColorMul2D/SivScopedColorMul2D.cpp
  ../Siv3D/src/Siv3D/ScopedRenderStates2D/SivScopedRenderStates2D.cpp
  ../Siv3D/src/Siv3D/ScopedScissorRect2D/SivScopedScissorRect2D.cpp
  ../Siv3D/src/Siv3D/ScopedViewport2D/SivScopedViewport2D.cpp
  ../Siv3D/src/Siv3D/Stopwatch/SivStopwatch.cpp
  ../Siv3D/src/Siv3D/String/Levenshtein.cpp
  ../Siv3D/src/Siv3D/String/SivString.cpp
  ../Siv3D/src/Siv3D/StringView/SivStringView.cpp
  ../Siv3D/src/Siv3D/StringView/SivStringView_SIMD.cpp
  # ../Siv3D/src/Siv3D/System/SivSystem.cpp
  # ../Siv3D/src/Siv3D/System/SystemFactory.cpp
  ../Siv3D/src/Siv3D/TextEncoding/SivTextEncoding.cpp
  ../Siv3D/src/Siv3D/TextReader/SivTextReader.cpp
  ../Siv3D/src/Siv3D/TextReader/TextReaderDetail.cpp
  ../Siv3D/src/Siv3D/TextWriter/SivTextWriter.cpp
  ../Siv3D/src/Siv3D/TextWriter/TextWriterDetail.cpp
  ../Siv3D/src/Siv3D/TextureAddressMode/SivTextureAddressMode.cpp
  ../Siv3D/src/Siv3D/TextureFilter/SivTextureFilter.cpp
  ../Siv3D/src/Siv3D/ThousandSeparate/SivThousandSeparate.cpp
  ../Siv3D/src/Siv3D/Threading/SivThreading.cpp
  ../Siv3D/src/Siv3D/Timer/SivTimer.cpp
  ../Siv3D/src/Siv3D/Transformer2D/SivTransformer2D.cpp
  ../Siv3D/src/Siv3D/Triangle/SivTriangle.cpp
  ../Siv3D/src/Siv3D/TriangleFillMode/SivTriangleFillMode.cpp
  ../Siv3D/src/Siv3D/Troubleshooting/Troubleshooting.cpp
  ../Siv3D/src/Siv3D/UUIDValue/SivUUIDValue.cpp
  ../Siv3D/src/Siv3D/Unicode/SivUnicode.cpp
  ../Siv3D/src/Siv3D/UnicodeConverter/SivUnicodeConverter.cpp
  ../Siv3D/src/Siv3D/UnicodeConverter/UnicodeUtility.cpp
  ../Siv3D/src/Siv3D/UserAction/CUserAction.cpp
  ../Siv3D/src/Siv3D/UserAction/UserActionFactory.cpp
  ../Siv3D/src/Siv3D/UserInfo/SivUserInfo.cpp
  ../Siv3D/src/Siv3D/VariableSpeedStopwatch/SivVariableSpeedStopwatch.cpp
  ../Siv3D/src/Siv3D/Vector2D/SivVector2D.cpp
  ../Siv3D/src/Siv3D/Vector3D/SivVector3D.cpp
  ../Siv3D/src/Siv3D/Vector4D/SivVector4D.cpp
  ../Siv3D/src/Siv3D/VertexShader/SivVertexShader.cpp
  # ../Siv3D/src/Siv3D/Window/SivWindow.cpp
  # ../Siv3D/src/Siv3D/Window/WindowFactory.cpp
  ../Siv3D/src/Siv3D/_TEMPLATE/_TEMPLATE.cpp


  ../Siv3D/src/ThirdParty/GLFW/cocoa_time.c
  ../Siv3D/src/ThirdParty/GLFW/context.c
  ../Siv3D/src/ThirdParty/GLFW/egl_context.c
  ../Siv3D/src/ThirdParty/GLFW/glx_context.c
  ../Siv3D/src/ThirdParty/GLFW/init.c
  ../Siv3D/src/ThirdParty/GLFW/input.c
  ../Siv3D/src/ThirdParty/GLFW/linux_joystick.c
  ../Siv3D/src/ThirdParty/GLFW/monitor.c
  ../Siv3D/src/ThirdParty/GLFW/null_init.c
  ../Siv3D/src/ThirdParty/GLFW/null_joystick.c
  ../Siv3D/src/ThirdParty/GLFW/null_monitor.c
  ../Siv3D/src/ThirdParty/GLFW/null_window.c
  ../Siv3D/src/ThirdParty/GLFW/osmesa_context.c
  ../Siv3D/src/ThirdParty/GLFW/platform.c
  ../Siv3D/src/ThirdParty/GLFW/posix_module.c
  ../Siv3D/src/ThirdParty/GLFW/posix_poll.c
  ../Siv3D/src/ThirdParty/GLFW/posix_thread.c
  ../Siv3D/src/ThirdParty/GLFW/posix_time.c
  ../Siv3D/src/ThirdParty/GLFW/vulkan.c
  ../Siv3D/src/ThirdParty/GLFW/wgl_context.c
  ../Siv3D/src/ThirdParty/GLFW/win32_init.c
  ../Siv3D/src/ThirdParty/GLFW/win32_joystick.c
  ../Siv3D/src/ThirdParty/GLFW/win32_module.c
  ../Siv3D/src/ThirdParty/GLFW/win32_monitor.c
  ../Siv3D/src/ThirdParty/GLFW/win32_thread.c
  ../Siv3D/src/ThirdParty/GLFW/win32_time.c
  ../Siv3D/src/ThirdParty/GLFW/win32_window.c
  ../Siv3D/src/ThirdParty/GLFW/window.c
  ../Siv3D/src/ThirdParty/GLFW/wl_init.c
  ../Siv3D/src/ThirdParty/GLFW/wl_monitor.c
  ../Siv3D/src/ThirdParty/GLFW/wl_window.c
  ../Siv3D/src/ThirdParty/GLFW/x11_init.c
  ../Siv3D/src/ThirdParty/GLFW/x11_monitor.c
  ../Siv3D/src/ThirdParty/GLFW/x11_window.c
  ../Siv3D/src/ThirdParty/GLFW/xkb_unicode.c

  ../Siv3D/src/ThirdParty/Oniguruma/ascii.c
  ../Siv3D/src/ThirdParty/Oniguruma/big5.c
  ../Siv3D/src/ThirdParty/Oniguruma/cp1251.c
  ../Siv3D/src/ThirdParty/Oniguruma/euc_jp.c
  ../Siv3D/src/ThirdParty/Oniguruma/euc_jp_prop.c
  ../Siv3D/src/ThirdParty/Oniguruma/euc_kr.c
  ../Siv3D/src/ThirdParty/Oniguruma/euc_tw.c
  ../Siv3D/src/ThirdParty/Oniguruma/gb18030.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_1.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_10.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_11.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_13.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_14.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_15.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_16.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_2.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_3.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_4.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_5.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_6.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_7.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_8.c
  ../Siv3D/src/ThirdParty/Oniguruma/iso8859_9.c
  ../Siv3D/src/ThirdParty/Oniguruma/koi8.c
  ../Siv3D/src/ThirdParty/Oniguruma/koi8_r.c
  ../Siv3D/src/ThirdParty/Oniguruma/onig_init.c
  ../Siv3D/src/ThirdParty/Oniguruma/regcomp.c
  ../Siv3D/src/ThirdParty/Oniguruma/regenc.c
  ../Siv3D/src/ThirdParty/Oniguruma/regerror.c
  ../Siv3D/src/ThirdParty/Oniguruma/regexec.c
  ../Siv3D/src/ThirdParty/Oniguruma/regext.c
  ../Siv3D/src/ThirdParty/Oniguruma/reggnu.c
  ../Siv3D/src/ThirdParty/Oniguruma/regparse.c
  ../Siv3D/src/ThirdParty/Oniguruma/regsyntax.c
  ../Siv3D/src/ThirdParty/Oniguruma/regtrav.c
  ../Siv3D/src/ThirdParty/Oniguruma/regversion.c
  ../Siv3D/src/ThirdParty/Oniguruma/sjis.c
  ../Siv3D/src/ThirdParty/Oniguruma/sjis_prop.c
  ../Siv3D/src/ThirdParty/Oniguruma/st.c
  ../Siv3D/src/ThirdParty/Oniguruma/unicode.c
  ../Siv3D/src/ThirdParty/Oniguruma/unicode_fold1_key.c
  ../Siv3D/src/ThirdParty/Oniguruma/unicode_fold2_key.c
  ../Siv3D/src/ThirdParty/Oniguruma/unicode_fold3_key.c
  ../Siv3D/src/ThirdParty/Oniguruma/unicode_unfold_key.c
  ../Siv3D/src/ThirdParty/Oniguruma/utf16_be.c
  ../Siv3D/src/ThirdParty/Oniguruma/utf16_le.c
  ../Siv3D/src/ThirdParty/Oniguruma/utf32_be.c
  ../Siv3D/src/ThirdParty/Oniguruma/utf32_le.c
  ../Siv3D/src/ThirdParty/Oniguruma/utf8.c

  ../Siv3D/src/ThirdParty/absl/numeric/int128.cc
  ../Siv3D/src/ThirdParty/absl/random/discrete_distribution.cc
  ../Siv3D/src/ThirdParty/absl/random/gaussian_distribution.cc

  ../Siv3D/src/ThirdParty/cpu_features/impl_x86_windows.c

  ../Siv3D/src/ThirdParty/double-conversion/bignum-dtoa.cc
  ../Siv3D/src/ThirdParty/double-conversion/bignum.cc
  ../Siv3D/src/ThirdParty/double-conversion/cached-powers.cc
  ../Siv3D/src/ThirdParty/double-conversion/double-to-string.cc
  ../Siv3D/src/ThirdParty/double-conversion/fast-dtoa.cc
  ../Siv3D/src/ThirdParty/double-conversion/fixed-dtoa.cc
  
  ../Siv3D/src/ThirdParty/easyexif/exif.cpp

  ../Siv3D/src/ThirdParty/fmt/format.cc

  ../Siv3D/src/ThirdParty/lunasvg/canvas.cpp
  ../Siv3D/src/ThirdParty/lunasvg/clippathelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/defselement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/element.cpp
  ../Siv3D/src/ThirdParty/lunasvg/gelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/geometryelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/graphicselement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/layoutcontext.cpp
  ../Siv3D/src/ThirdParty/lunasvg/lunasvg.cpp
  ../Siv3D/src/ThirdParty/lunasvg/markerelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/maskelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/paintelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/parser.cpp
  ../Siv3D/src/ThirdParty/lunasvg/property.cpp
  ../Siv3D/src/ThirdParty/lunasvg/stopelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/styledelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/styleelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/svgelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/symbolelement.cpp
  ../Siv3D/src/ThirdParty/lunasvg/useelement.cpp

  ../Siv3D/src/ThirdParty/muparser/muParser.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserBase.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserBytecode.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserCallback.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserError.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserInt.cpp
  ../Siv3D/src/ThirdParty/muparser/muParserTokenReader.cpp

  ../Siv3D/src/ThirdParty/plutovg/plutovg-blend.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-dash.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-ft-math.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-ft-raster.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-ft-stroker.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-geometry.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-paint.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg-rle.c
  ../Siv3D/src/ThirdParty/plutovg/plutovg.c

  ../Siv3D/src/ThirdParty/simdutf/simdutf.cpp

  ../Siv3D/src/ThirdParty/zstd/common/debug.c
  ../Siv3D/src/ThirdParty/zstd/common/entropy_common.c
  ../Siv3D/src/ThirdParty/zstd/common/error_private.c
  ../Siv3D/src/ThirdParty/zstd/common/fse_decompress.c
  ../Siv3D/src/ThirdParty/zstd/common/pool.c
  ../Siv3D/src/ThirdParty/zstd/common/threading.c
  ../Siv3D/src/ThirdParty/zstd/common/xxhash.c
  ../Siv3D/src/ThirdParty/zstd/common/zstd_common.c
  ../Siv3D/src/ThirdParty/zstd/compress/fse_compress.c
  ../Siv3D/src/ThirdParty/zstd/compress/hist.c
  ../Siv3D/src/ThirdParty/zstd/compress/huf_compress.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_compress.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_compress_literals.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_compress_sequences.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_compress_superblock.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_double_fast.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_fast.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_lazy.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_ldm.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstd_opt.c
  ../Siv3D/src/ThirdParty/zstd/compress/zstdmt_compress.c
  ../Siv3D/src/ThirdParty/zstd/decompress/huf_decompress.c
  ../Siv3D/src/ThirdParty/zstd/decompress/huf_decompress_amd64.S
  ../Siv3D/src/ThirdParty/zstd/decompress/zstd_ddict.c
  ../Siv3D/src/ThirdParty/zstd/decompress/zstd_decompress.c
  ../Siv3D/src/ThirdParty/zstd/decompress/zstd_decompress_block.c
  ../Siv3D/src/ThirdParty/zstd/dictBuilder/cover.c
  ../Siv3D/src/ThirdParty/zstd/dictBuilder/divsufsort.c
  ../Siv3D/src/ThirdParty/zstd/dictBuilder/fastcover.c
  ../Siv3D/src/ThirdParty/zstd/dictBuilder/zdict.c
)

set(SIV3D_SKIA_SOURCE
  ../Siv3D/src/ThirdParty/skia/modules/skcms/skcms.cc
  ../Siv3D/src/ThirdParty/skia/modules/skcms/src/skcms_TransformBaseline.cc
  ../Siv3D/src/ThirdParty/skia/modules/skcms/src/skcms_TransformHsw.cc
  ../Siv3D/src/ThirdParty/skia/modules/skcms/src/skcms_TransformSkx.cc
  ../Siv3D/src/ThirdParty/skia/src/base/SkArenaAlloc.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkBezierCurves.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkBlockAllocator.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkBuffer.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkContainers.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkCubics.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkDebug.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkDeque.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkFloatingPoint.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkHalf.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkMalloc.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkMathPriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkQuads.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkSafeMath.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkSemaphore.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkSharedMutex.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkSpinlock.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkTDArray.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkThreadID.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkTSearch.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkUTF.cpp
  ../Siv3D/src/ThirdParty/skia/src/base/SkUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/codec/SkCodec.cpp
  ../Siv3D/src/ThirdParty/skia/src/codec/SkCodecImageGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/codec/SkImageGenerator_FromEncoded.cpp
  ../Siv3D/src/ThirdParty/skia/src/codec/SkPixmapUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/codec/SkSampler.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkAAClip.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkAlphaRuns.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkAnalyticEdge.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkAutoPixmapStorage.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBBHFactory.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBigPicture.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmap.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapDevice.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapProcState.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapProcState_matrixProcs.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapProcState_opts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBitmapProcState_opts_ssse3.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlendMode.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlendModeBlender.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitMask_opts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitMask_opts_ssse3.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitRow_D32.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitRow_opts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitRow_opts_hsw.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitter_A8.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitter_ARGB32.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlitter_Sprite.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlurMask.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkBlurMaskFilterImpl.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCachedData.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCanvas.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCanvasPriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCanvas_Raster.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCapabilities.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkChecksum.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkClipStack.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkColor.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkColorSpace.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkColorSpaceXformSteps.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkColorTable.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCompressedDataUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkContourMeasure.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkConvertPixels.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCpu.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCubicClipper.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkCubicMap.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkData.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDescriptor.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDevice.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDraw.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDrawable.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDrawBase.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDrawShadowInfo.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDraw_atlas.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDraw_text.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkDraw_vertices.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkEdge.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkEdgeBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkEdgeClipper.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkExecutor.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFlattenable.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFont.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFontDescriptor.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFontMetricsPriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFontMgr.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkFont_serial.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGaussFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGeometry.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGlobalInitialization_core.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGlyph.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGlyphRunPainter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkGraphics.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkIDChangeListener.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkImageFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkImageFilterCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkImageFilterTypes.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkImageGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkImageInfo.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkKnownRuntimeEffects.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkLatticeIter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkLineClipper.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkLocalMatrixImageFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkM44.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMallocPixelRef.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMask.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMaskBlurFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMaskCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMaskFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMaskGamma.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMasks.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMatrix.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMatrixInvert.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMemset_opts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMemset_opts_avx.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMemset_opts_erms.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMesh.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMipmap.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMipmapAccessor.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMipmapBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMipmapDrawDownSampler.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkMipmapHQDownSampler.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkOpts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPaint.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPaintPriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPath.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPathBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPathEffect.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPathMeasure.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPathRef.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPathUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPath_serial.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPicture.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPictureData.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPictureFlat.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPicturePlayback.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPictureRecord.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPictureRecorder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPixelRef.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPixmap.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPixmapDraw.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPoint.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPoint3.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkPtrRecorder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkQuadClipper.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRasterClip.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRasterPipeline.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRasterPipelineBlitter.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkReadBuffer.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkReadPixelsRec.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecord.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecordDraw.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecordedDrawable.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecorder.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecordOpts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRecords.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRect.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRegion.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRegion_path.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkResourceCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRRect.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRSXform.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRTree.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRuntimeBlender.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkRuntimeEffect.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScalar.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScalerContext.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan_AAAPath.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan_Antihair.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan_AntiPath.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan_Hairline.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkScan_Path.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSpecialImage.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSpriteBlitter_ARGB32.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStream.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStrike.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStrikeCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStrikeSpec.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkString.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStringUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStroke.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStrokeRec.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkStrokerPriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSwizzle.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSwizzler_opts.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSwizzler_opts_hsw.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkSwizzler_opts_ssse3.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkTaskGroup.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkTextBlob.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkTypeface.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkTypefaceCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkUnPreMultiply.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkVertices.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkVertState.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkWriteBuffer.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkWritePixelsRec.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkWriter32.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkYUVAInfo.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkYUVAPixmaps.cpp
  ../Siv3D/src/ThirdParty/skia/src/core/SkYUVPlanesCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkBlendModeColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkColorFilterBase.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkComposeColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkRuntimeColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkWorkingFormatColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/colorfilters/SkGaussianColorFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/imagefilters/SkBlendImageFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/imagefilters/SkBlurImageFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/imagefilters/SkCropImageFilter.cpp
  ../Siv3D/src/ThirdParty/skia/src/effects/SkBlenders.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkImage.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkImage_Base.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkImage_Lazy.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkImage_Raster.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkImage_RasterFactories.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkRescaleAndReadPixels.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkSurface.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkSurface_Base.cpp
  ../Siv3D/src/ThirdParty/skia/src/image/SkSurface_Raster.cpp
  ../Siv3D/src/ThirdParty/skia/src/opts/SkOpts_hsw.cpp
  ../Siv3D/src/ThirdParty/skia/src/opts/SkOpts_skx.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkAddIntersections.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkDConicLineIntersection.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkDCubicLineIntersection.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkDCubicToQuads.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkDLineIntersection.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkDQuadLineIntersection.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkIntersections.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpAngle.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpCoincidence.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpContour.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpCubicHull.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpEdgeBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpSegment.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkOpSpan.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsAsWinding.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsCommon.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsConic.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsCubic.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsCurve.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsDebug.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsLine.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsOp.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsQuad.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsRect.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsSimplify.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsTightBounds.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsTSect.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsTypes.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathOpsWinding.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkPathWriter.cpp
  ../Siv3D/src/ThirdParty/skia/src/pathops/SkReduceOrder.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkDebug_stdio.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkFontHost_FreeType.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkFontHost_FreeType_common.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkGlobalInitialization_default.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkMemory_malloc.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkOSFile_stdio.cpp
  ../Siv3D/src/ThirdParty/skia/src/ports/SkOSFile_posix.cpp
  ../Siv3D/src/ThirdParty/skia/src/sfnt/SkOTTable_name.cpp
  ../Siv3D/src/ThirdParty/skia/src/sfnt/SkOTUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/gradients/SkConicalGradient.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/gradients/SkGradientBaseShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/gradients/SkLinearGradient.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/gradients/SkRadialGradient.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/gradients/SkSweepGradient.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkBitmapProcShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkBlendShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkColorFilterShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkColorShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkEmptyShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkImageShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkLocalMatrixShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkRuntimeShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkShaderBase.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkTransformShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkTriColorShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/shaders/SkWorkingColorSpaceShader.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLCanExitWithoutReturningValue.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLCheckProgramStructure.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLCheckSymbolTableCorrectness.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLFinalizationChecks.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLGetLoopControlFlowInfo.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLGetLoopUnrollInfo.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLGetReturnComplexity.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLHasSideEffects.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLIsConstantExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLIsDynamicallyUniformExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLIsSameExpressionTree.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLIsTrivialExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLProgramUsage.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLReturnsInputAlpha.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLSwitchCaseContainsExit.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/analysis/SkSLSymbolTableStackBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLGLSLCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLHLSLCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLMetalCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLPipelineStageCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLRasterPipelineBuilder.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLRasterPipelineCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLSPIRVCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLSPIRVtoHLSL.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/codegen/SkSLWGSLCodeGenerator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLBinaryExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLBlock.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLChildCall.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructor.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorArray.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorArrayCast.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorCompound.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorCompoundCast.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorDiagonalMatrix.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorMatrixResize.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorScalarCast.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorSplat.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLConstructorStruct.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLDiscardStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLDoStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLExpressionStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLExtension.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLFieldAccess.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLForStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLFunctionCall.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLFunctionDeclaration.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLFunctionDefinition.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLIfStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLIndexExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLInterfaceBlock.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLLayout.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLLiteral.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLModifierFlags.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLModifiersDeclaration.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLPostfixExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLPrefixExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLProgram.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSetting.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLStructDefinition.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSwitchCase.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSwitchStatement.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSwizzle.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSymbol.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLSymbolTable.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLTernaryExpression.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLType.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLTypeReference.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLVarDeclarations.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLVariable.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/ir/SkSLVariableReference.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLAnalysis.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLBuiltinTypes.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLCompiler.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLConstantFolder.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLContext.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLErrorReporter.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLInliner.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLIntrinsicList.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLLexer.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLMangler.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLModuleLoader.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLOperator.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLOutputStream.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLParser.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLPool.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLPosition.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLSampleUsage.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLString.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/SkSLUtil.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/tracing/SkSLDebugTracePlayer.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/tracing/SkSLDebugTracePriv.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/tracing/SkSLTraceHook.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLAddConstToVarModifiers.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateDeadFunctions.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateDeadGlobalVariables.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateDeadLocalVariables.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateEmptyStatements.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateUnnecessaryBraces.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLEliminateUnreachableCode.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLFindAndDeclareBuiltinFunctions.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLFindAndDeclareBuiltinStructs.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLFindAndDeclareBuiltinVariables.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLHoistSwitchVarDeclarationsAtTopLevel.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLRenamePrivateSymbols.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLReplaceConstVarsWithLiterals.cpp
  ../Siv3D/src/ThirdParty/skia/src/sksl/transform/SkSLRewriteIndexedSwizzle.cpp
  ../Siv3D/src/ThirdParty/skia/src/text/GlyphRun.cpp
  ../Siv3D/src/ThirdParty/skia/src/text/SlugFromBuffer.cpp
  ../Siv3D/src/ThirdParty/skia/src/text/StrikeForGPU.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkCharToGlyphCache.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkCustomTypeface.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkDashPath.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkEventTracer.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkJSON.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkJSONWriter.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkMatrix22.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkParse.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkPatchUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkPolyUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkShadowTessellator.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkShadowUtils.cpp
  ../Siv3D/src/ThirdParty/skia/src/utils/SkTextUtils.cpp
)

set(SIV3D_INTERNAL_INCLUDE_DIRS
  ../Siv3D/include
  ../Siv3D/include/ThirdParty
  ../Siv3D/src
  ../Siv3D/src/Siv3D-Platform/Linux
  ../Siv3D/src/Siv3D-Platform/macOS_Linux
  ../Siv3D/src/ThirdParty
  ../Siv3D/src/ThirdParty/skia
  ${SIV3D_THIRD_PARTY_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_library(Siv3D_Skia OBJECT ${SIV3D_SKIA_SOURCE})
target_include_directories(Siv3D_Skia PRIVATE
  ../Siv3D/src/ThirdParty
  ../Siv3D/src/ThirdParty/skia
  ../Siv3D/src/ThirdParty-prebuilt/freetype
)
target_compile_definitions(Siv3D_Skia PRIVATE
  SK_DISABLE_LEGACY_PNG_WRITEBUFFER
  SK_DISABLE_EFFECT_DESERIALIZATION
)
target_compile_options(Siv3D_Skia PRIVATE
  -mavx512f -mavx512dq -mfma
  ${SIV3D_CXX_FLAGS} ${SIV3D_CXX_FLAGS_WITH_COMPILER}
    $<$<CONFIG:Debug>:   ${SIV3D_CXX_FLAGS_DEBUG}>
    $<$<CONFIG:Release>: ${SIV3D_CXX_FLAGS_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>: ${SIV3D_CXX_FLAGS_RELWITHDEBINFO}>
    $<$<CONFIG:MinSizeRel>: ${SIV3D_CXX_FLAGS_MINSIZEREL}>
)

add_library(Siv3D STATIC ${SIV3D_INTERNAL_SOURCES} $<TARGET_OBJECTS:Siv3D_Skia>)
target_link_libraries(Siv3D INTERFACE
  ${CMAKE_THREAD_LIBS_INIT}
  ${GIF_LIBRARIES}
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

