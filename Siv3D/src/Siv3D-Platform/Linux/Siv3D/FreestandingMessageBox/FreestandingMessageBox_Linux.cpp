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

# include <iostream>
# include <Siv3D/FreestandingMessageBox/FreestandingMessageBox.hpp>

namespace s3d
{
	namespace FreestandingMessageBox
	{
		// Linux にはまだ GUI バックエンドが無いため、標準エラー出力にフォールバックする。
		// (FreestandingMessageBox はエンジン初期化前の致命的エラー通知に使われる)

		void ShowInfo(const std::string_view text)
		{
			std::cerr << "[Siv3D] " << text << '\n';
		}

		void ShowError(const std::string_view text)
		{
			std::cerr << "[Siv3D] Error: " << text << '\n';
		}
	}
}
