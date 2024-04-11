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
	//	ImageFormat
	//
	////////////////////////////////////////////////////////////////

	/// @brief 画像のエンコード形式
	enum class ImageFormat
	{
		/// @brief 不明
		Unknown,

		/// @brief DDS
		DDS,

		/// @brief PNG
		PNG,

		/// @brief JPEG
		JPEG,

		/// @brief JPEG 2000
		JPEG2000,

		/// @brief JPEG XL
		JPEG_XL,

		/// @brief BMP
		BMP,

		/// @brief WebP
		WebP,

		/// @brief GIF
		GIF,

		/// @brief TIFF
		TIFF,

		/// @brief TGA
		TGA,

		/// @brief PPM
		PPM,

		/// @brief SVG
		SVG,

		/// @brief 指定しない（データと拡張子から判断）
		Unspecified = Unknown,
	};
}