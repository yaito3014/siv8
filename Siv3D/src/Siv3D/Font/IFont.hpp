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
# include <Siv3D/Common.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/Font.hpp>
# include <Siv3D/FontFaceProperties.hpp>

namespace s3d
{
	struct FontFaceInfo;

	class SIV3D_NOVTABLE ISiv3DFont
	{
	public:

		[[nodiscard]]
		static ISiv3DFont* Create();

		virtual ~ISiv3DFont() = default;

		virtual void init() = 0;

		[[nodiscard]]
		virtual Array<FontFaceProperties> getFontFaces(FilePathView path) const = 0;

		[[nodiscard]]
		virtual Font::IDType create(FilePathView path, size_t faceIndex, StringView styleName, FontMethod fontMethod, int32 baseSize, FontStyle style) = 0;

		virtual void release(Font::IDType handleID) = 0;

		[[nodiscard]]
		virtual const FontFaceInfo& getInfo(Font::IDType handleID) = 0;

		virtual void setTabSize(Font::IDType handleID, int32 tabSize) = 0;

		[[nodiscard]]
		virtual GlyphIndex getGlyphIndex(Font::IDType handleID, char32 codePoint) = 0;

		[[nodiscard]]
		virtual GlyphIndex getGlyphIndex(Font::IDType handleID, StringView ch) = 0;

		[[nodiscard]]
		virtual String getGlyphNameByGlyphIndex(Font::IDType handleID, GlyphIndex glyphIndex) = 0;
		
		[[nodiscard]]
		virtual double getXAdvanceFromGlyphIndex(Font::IDType handleID, GlyphIndex glyphIndex) = 0;
	};
}
