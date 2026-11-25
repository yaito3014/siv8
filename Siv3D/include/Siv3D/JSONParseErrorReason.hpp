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
# include "Common.hpp"
# include "String.hpp"
# include "Array.hpp"

namespace s3d
{
	/// @brief JSON の読み込みやパースに失敗した理由
	struct JSONParseErrorReason
	{
		/// @brief エラーの説明
		String title;

		/// @brief エラーの位置。`入力名:行:列` の形式で、位置がない場合は空
		Array<String> locations;

		/// @brief 解決のヒント。ない場合は空
		String hint;

		/// @brief title, locations, hint を改行で区切った文字列を返します。空の要素は含めません。
		/// @return フォーマットした文字列
		[[nodiscard]]
		String format() const;
	};
}
