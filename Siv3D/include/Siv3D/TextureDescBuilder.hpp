﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2025 Ryo Suzuki
//	Copyright (c) 2016-2025 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "Common.hpp"

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	TextureDescBuilder
	//
	////////////////////////////////////////////////////////////////

	class TextureDescBuilder
	{
	public:

		////////////////////////////////////////////////////////////////
		//
		//	(constructor)
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		explicit constexpr TextureDescBuilder(
			bool _hasMipmap = true,
			bool _sRGB = true,
			bool _isSDF = false
		) noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	hasMipmap
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		constexpr bool hasMipmap() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	sRGB
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		constexpr bool sRGB() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	isSDF
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		constexpr bool isSDF() const noexcept;

	private:
		
		bool m_hasMipmap	: 1 = true;

		bool m_sRGB			: 1 = true;

		bool m_isSDF		: 1 = false;
	};
}

# include "detail/TextureDescBuilder.ipp"
