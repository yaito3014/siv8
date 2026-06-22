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
# include <Siv3D/Clipboard/IClipboard.hpp>

namespace s3d
{
	// TODO(linux): no clipboard backend yet (would use GLFW/X11 clipboard).
	class CClipboard final : public ISiv3DClipboard
	{
	public:

		void init() override {}

		bool hasChanged() override { return false; }

		uint64 getSequenceNumber() override { return 0; }

		void clear() override {}

		void setText(StringView) override {}

		bool getText(String&) override { return false; }

		bool hasText() override { return false; }

		void setImage(const Image&) override {}

		bool getImage(Image&, PremultiplyAlpha) override { return false; }

		bool hasImage() override { return false; }

		void setFilePaths(const Array<FilePath>&) override {}

		bool getFilePaths(Array<FilePath>&) override { return false; }

		bool hasFilePaths() override { return false; }

		void setRichText(StringView, const Optional<StringView>&) override {}

		void setHTML(StringView, const Optional<StringView>&) override {}

		void setData(StringView, const void*, size_t, const Optional<StringView>&) override {}

		bool getData(StringView, Blob&) override { return false; }

		Array<String> getAvailableMimeTypes() override { return{}; }
	};
}
