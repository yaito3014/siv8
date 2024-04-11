﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2024 Ryo Suzuki
//	Copyright (c) 2016-2024 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	ImagePixelFormat
	//
	////////////////////////////////////////////////////////////////

	/// @brief 画像のピクセルフォーマット
	enum class ImagePixelFormat
	{
		/// @brief 不明
		Unknown,

		/// @brief R 8-bit, G 8-bit, B 8-bit
		R8G8B8,

		/// @brief R 8-bit, G 8-bit, B 8-bit, X 8-bit
		R8G8B8X8,

		/// @brief R 8-bit, G 8-bit, B 8-bit, A 8-bit
		R8G8B8A8,

		/// @brief Gray 8-bit
		Gray8,

		/// @brief Gray 16-bit
		Gray16,

		/// @brief Gray 8-bit, A 8-bit
		Gray8A8,

		/// @brief Gray 16-bit, A 16-bit
		Gray16A16,
	};
}