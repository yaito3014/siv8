//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/Texture/ITexture.hpp>

namespace s3d
{
	// TODO(linux): real GL texture management. Phase 0 no-op.
	class CTexture_GL4 final : public ISiv3DTexture
	{
	public:

		void init() override {}
		Texture::IDType create(std::unique_ptr<IReader> reader, FilePathView pathHint, TextureDesc desc) override { return {}; }
		Texture::IDType create(const Image& image, const Array<Image>& mipmaps, TextureDesc desc) override { return {}; }
		Texture::IDType create(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override { return {}; }
		Texture::IDType create(const BCnData& bcnData) override { return {}; }
		Texture::IDType createDynamic(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override { return {}; }
		void release(Texture::IDType handleID) override {}
		Size getSize(Texture::IDType handleID) override { return {}; }
		uint32 getMipLevels(Texture::IDType handleID) override { return {}; }
		TextureDesc getDesc(Texture::IDType handleID) override { return {}; }
		TextureFormat getFormat(Texture::IDType handleID) override { return {}; }
		bool hasDepth(Texture::IDType handleID) override { return {}; }
		bool fill(Texture::IDType handleID, const ColorF& color, bool wait) override { return {}; }
		bool fill(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, bool wait) override { return {}; }
		bool fillRegion(Texture::IDType handleID, const ColorF& color, const Rect& rect) override { return {}; }
		bool fillRegion(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, const Rect& rect, bool wait) override { return {}; }
		void generateMips(Texture::IDType handleID) override {}
	};
}
