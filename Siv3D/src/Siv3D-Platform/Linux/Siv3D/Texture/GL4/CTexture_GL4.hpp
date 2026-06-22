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
# include <Siv3D/PointVector.hpp>
# include <Siv3D/TextureDesc.hpp>
# include <Siv3D/TextureFormat.hpp>
# include <Siv3D/AssetHandleManager/AssetHandleManager.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// Phase 1: a minimal GL texture backend (RGBA8). Enough for sprites and the
	// font glyph atlas; compressed (BCn) / sRGB / mipmaps are TODO(linux).
	class CTexture_GL4 final : public ISiv3DTexture
	{
	public:

		struct GLTexture
		{
			GLuint texture = 0;
			Size size{ 0, 0 };
			TextureDesc desc{};
			TextureFormat format{};
			~GLTexture() { if (texture) { ::glDeleteTextures(1, &texture); } }
		};

		~CTexture_GL4() override;

		void init() override {}

		Texture::IDType create(std::unique_ptr<IReader> reader, FilePathView pathHint, TextureDesc desc) override;
		Texture::IDType create(const Image& image, const Array<Image>& mipmaps, TextureDesc desc) override;
		Texture::IDType create(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override;
		Texture::IDType create(const BCnData& bcnData) override { return Texture::IDType::Null(); }
		Texture::IDType createDynamic(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override;

		void release(Texture::IDType handleID) override;

		Size getSize(Texture::IDType handleID) override;
		uint32 getMipLevels(Texture::IDType handleID) override { return 1; }
		TextureDesc getDesc(Texture::IDType handleID) override;
		TextureFormat getFormat(Texture::IDType handleID) override;
		bool hasDepth(Texture::IDType handleID) override { return false; }

		bool fill(Texture::IDType handleID, const ColorF& color, bool wait) override { return false; }
		bool fill(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, bool wait) override;
		bool fillRegion(Texture::IDType handleID, const ColorF& color, const Rect& rect) override { return false; }
		bool fillRegion(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, const Rect& rect, bool wait) override;
		void generateMips(Texture::IDType handleID) override {}

		// Linux renderer access: GL texture name for a handle (0 if none).
		[[nodiscard]]
		GLuint getGLTexture(Texture::IDType handleID);

	private:

		AssetHandleManager<Texture::IDType, GLTexture> m_textures{ "Texture" };
	};
}
