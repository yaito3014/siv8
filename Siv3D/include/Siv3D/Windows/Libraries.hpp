//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2026 Ryo Suzuki
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "../Platform.hpp"

# if (SIV3D_PLATFORM(WINDOWS) && !defined(SIV3D_LIBRARY_BUILD))

	// エンジン本体とサードパーティライブラリは CMake / vcpkg のターゲットとして
	// リンクされる。ここでは vcpkg が面倒を見ない Windows SDK のシステム
	// ライブラリだけを自動リンクする。
	# pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
	# pragma comment (lib, "dwmapi")
	# pragma comment (lib, "mfplat")
	# pragma comment (lib, "mfuuid")
	# pragma comment (lib, "mincore")
	# pragma comment (lib, "Secur32")
	# pragma comment (lib, "setupapi")
	# pragma comment (lib, "winmm")
	# pragma comment (lib, "wininet")

# endif // (SIV3D_PLATFORM(WINDOWS) && !defined(SIV3D_LIBRARY_BUILD))
