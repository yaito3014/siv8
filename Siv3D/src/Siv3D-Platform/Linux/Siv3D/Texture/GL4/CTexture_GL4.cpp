//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CTexture_GL4.hpp"
# include <cstring>
# include <Siv3D/Image.hpp>
# include <Siv3D/2DShapes.hpp>
# include <Siv3D/BCnData.hpp>
# include <Siv3D/ImageProcessing.hpp>
# include <Siv3D/ImageFormat/BCnDecoder.hpp>
# include <Siv3D/Texture/TextureUtility.hpp>
# include <Siv3D/Error/InternalEngineError.hpp>
# include <Siv3D/FormatLiteral.hpp>
# include <Siv3D/EngineLog.hpp>

namespace s3d
{
	namespace
	{
		[[nodiscard]]
		static Size MipSize(const Size& size, const uint32 level) noexcept
		{
			return{ Max((size.x >> level), 1), Max((size.y >> level), 1) };
		}

		[[nodiscard]]
		static bool IsUncompressedFormat(const TextureFormat& format) noexcept
		{
			return ((not format.isBCn()) && (format.GLInternalFormat() != 0) && (format.pixelSize() != 0));
		}

		// Creates a bound texture object whose sampling covers exactly `mipLevels` levels, so a
		// mipmapped sampler never reads an unspecified level.
		[[nodiscard]]
		static GLuint CreateTextureObject(const uint32 mipLevels)
		{
			GLuint texture = 0;
			::glGenTextures(1, &texture);
			::glBindTexture(GL_TEXTURE_2D, texture);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mipLevels - 1));
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ((1 < mipLevels) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			return texture;
		}

		// Rows of uncompressed sources are TextureFormat::bytesPerRow(), i.e. padded to 4 bytes.
		static void SpecifyLevel(const TextureFormat& format, const uint32 level, const Size& size, const void* data)
		{
			::glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			::glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), format.GLInternalFormat(), size.x, size.y, 0,
				static_cast<GLenum>(format.GLFormat()), static_cast<GLenum>(format.GLType()), data);
		}

		[[nodiscard]]
		static std::unique_ptr<CTexture_GL4::GLTexture> MakeTexture(const Size& size, const uint32 mipLevels,
			const TextureFormat& format, const TextureDesc desc, const bool dynamic)
		{
			auto texture		= std::make_unique<CTexture_GL4::GLTexture>();
			texture->texture	= CreateTextureObject(mipLevels);
			texture->size		= size;
			texture->mipLevels	= mipLevels;
			texture->desc		= desc;
			texture->format		= format;
			texture->dynamic	= dynamic;
			return texture;
		}

		[[nodiscard]]
		static std::unique_ptr<CTexture_GL4::GLTexture> CreateFromImage(const Image& image, const Array<Image>& mipmaps, const TextureDesc desc)
		{
			const TextureFormat format = (desc.sRGB ? TextureFormat::R8G8B8A8_Unorm_SRGB : TextureFormat::R8G8B8A8_Unorm);
			const bool useMipmaps = (desc.hasMipmap && (image.size() != Size{ 1, 1 }) && (not mipmaps.isEmpty()));
			const uint32 mipLevels = (useMipmaps ? static_cast<uint32>(mipmaps.size() + 1) : 1);

			auto texture = MakeTexture(image.size(), mipLevels, format, desc, false);
			SpecifyLevel(format, 0, image.size(), image.data());

			if (useMipmaps)
			{
				for (uint32 i = 0; i < mipmaps.size(); ++i)
				{
					SpecifyLevel(format, (i + 1), mipmaps[i].size(), mipmaps[i].data());
				}
			}

			::glBindTexture(GL_TEXTURE_2D, 0);
			return texture;
		}

		[[nodiscard]]
		static std::unique_ptr<CTexture_GL4::GLTexture> CreateFromData(const Size& size, const std::span<const Byte> data,
			const TextureFormat& format, const TextureDesc desc, const bool dynamic)
		{
			const bool generateMipmaps = (desc.hasMipmap && (size != Size{ 1, 1 }));
			const uint32 mipLevels = (generateMipmaps ? ImageProcessing::CalculateMipmapLevel(size.x, size.y) : 1);

			auto texture = MakeTexture(size, mipLevels, format, desc, dynamic);
			SpecifyLevel(format, 0, size, (data.empty() ? nullptr : data.data()));

			for (uint32 level = 1; level < mipLevels; ++level)
			{
				SpecifyLevel(format, level, MipSize(size, level), nullptr);
			}

			if (generateMipmaps && (not data.empty()))
			{
				::glGenerateMipmap(GL_TEXTURE_2D);
			}

			::glBindTexture(GL_TEXTURE_2D, 0);
			return texture;
		}

		[[nodiscard]]
		static bool IsValidRegion(const CTexture_GL4::GLTexture& texture, const Rect& rect) noexcept
		{
			return (texture.dynamic
				&& (0 <= rect.x)
				&& (0 <= rect.y)
				&& (0 < rect.w)
				&& (0 < rect.h)
				&& (rect.w <= texture.size.x)
				&& (rect.h <= texture.size.y)
				&& (rect.x <= (texture.size.x - rect.w))
				&& (rect.y <= (texture.size.y - rect.h)));
		}

		static void UploadRegion(const CTexture_GL4::GLTexture& texture, const Rect& rect, const GLint alignment, const GLint rowLength, const void* data)
		{
			::glBindTexture(GL_TEXTURE_2D, texture.texture);
			::glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
			::glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
			::glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.w, rect.h,
				static_cast<GLenum>(texture.format.GLFormat()), static_cast<GLenum>(texture.format.GLType()), data);
			::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			::glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Uploads rows that are `srcBytesPerRow` apart. GL can stride over whole pixels itself;
		// any other pitch is repacked into the texture's staging buffer first.
		[[nodiscard]]
		static bool UploadRows(CTexture_GL4::GLTexture& texture, const Rect& rect, const std::span<const Byte> src, const uint32 srcBytesPerRow)
		{
			const uint32 pixelSize = texture.format.pixelSize();
			const size_t copyBytesPerRow = (static_cast<size_t>(rect.w) * pixelSize);

			if ((pixelSize == 0)
				|| (srcBytesPerRow < copyBytesPerRow)
				|| (src.size_bytes() < (static_cast<size_t>(srcBytesPerRow) * rect.h)))
			{
				return false;
			}

			if ((srcBytesPerRow % pixelSize) == 0)
			{
				UploadRegion(texture, rect, 1, static_cast<GLint>(srcBytesPerRow / pixelSize), src.data());
				return true;
			}

			const uint32 bytesPerRow = texture.format.bytesPerRow(rect.w);
			texture.staging.resize(static_cast<size_t>(bytesPerRow) * rect.h);

			Byte* pDst = texture.staging.data();
			const Byte* pSrc = src.data();

			for (int32 y = 0; y < rect.h; ++y)
			{
				std::memcpy(pDst, pSrc, copyBytesPerRow);
				pDst += bytesPerRow;
				pSrc += srcBytesPerRow;
			}

			UploadRegion(texture, rect, 4, 0, texture.staging.data());
			return true;
		}

		static void FillRegionWithColor(CTexture_GL4::GLTexture& texture, const Rect& rect, const ColorF& color)
		{
			const size_t totalBytes = (static_cast<size_t>(texture.format.bytesPerRow(rect.w)) * rect.h);
			texture.staging.resize(totalBytes);
			FillWithColor(texture.staging.data(), totalBytes, color, texture.format);
			UploadRegion(texture, rect, 4, 0, texture.staging.data());
		}
	}

	CTexture_GL4::GLTexture::~GLTexture()
	{
		if (texture)
		{
			::glDeleteTextures(1, &texture);
		}
	}

	CTexture_GL4::~CTexture_GL4()
	{
		LOG_SCOPED_DEBUG("CTexture_GL4::~CTexture_GL4()");

		m_textures.destroy();
	}

	void CTexture_GL4::init()
	{
		LOG_SCOPED_DEBUG("CTexture_GL4::init()");

		m_s3tcSupported		= HasGLExtension("GL_EXT_texture_compression_s3tc");
		m_s3tcSRGBSupported	= (m_s3tcSupported
			&& (HasGLExtension("GL_EXT_texture_sRGB") || HasGLExtension("GL_EXT_texture_compression_s3tc_srgb")));
		m_bptcSupported		= (((4 < GLVersion.major) || ((GLVersion.major == 4) && (2 <= GLVersion.minor)))
			|| HasGLExtension("GL_ARB_texture_compression_bptc"));

		LOG_INFO(fmt::format("ℹ️ GL BCn support: S3TC {}, S3TC sRGB {}, BPTC {}", m_s3tcSupported, m_s3tcSRGBSupported, m_bptcSupported));

		const Image image{ 16, Palette::Yellow };
		const Array<Image> mipmaps = {
			Image{ 8, Palette::Yellow }, Image{ 4, Palette::Yellow },
			Image{ 2, Palette::Yellow }, Image{ 1, Palette::Yellow }
		};

		auto nullTexture = CreateFromImage(image, mipmaps, TextureDesc::Mipmap);

		if (not nullTexture->texture)
		{
			throw InternalEngineError{ "Failed to create a null texture" };
		}

		m_textures.setNullData(std::move(nullTexture));
	}

	Texture::IDType CTexture_GL4::create(std::unique_ptr<IReader> reader, const FilePathView pathHint, const TextureDesc desc)
	{
		if ((not reader) || (not reader->isOpen()))
		{
			return Texture::IDType::Null();
		}

		const BCnDecoder bcnDecoder{};

		if (bcnDecoder.getImageInfo(*reader).has_value())
		{
			return create(bcnDecoder.decodeNative(std::move(reader), desc.sRGB, pathHint));
		}

		const Image image{ std::move(reader) };

		if (not image)
		{
			return Texture::IDType::Null();
		}

		return create(image.size(), std::as_bytes(std::span{ image }), (desc.sRGB ? TextureFormat::R8G8B8A8_Unorm_SRGB : TextureFormat::R8G8B8A8_Unorm), desc);
	}

	Texture::IDType CTexture_GL4::create(const Image& image, const Array<Image>& mipmaps, const TextureDesc desc)
	{
		if (not image)
		{
			return Texture::IDType::Null();
		}

		return add(CreateFromImage(image, mipmaps, desc));
	}

	Texture::IDType CTexture_GL4::create(const Size& size, const std::span<const Byte> data, const TextureFormat& format, const TextureDesc desc)
	{
		if ((size.x <= 0) || (size.y <= 0) || (not IsUncompressedFormat(format))
			|| ((not data.empty()) && (data.size_bytes() < (static_cast<size_t>(format.bytesPerRow(size.x)) * size.y))))
		{
			return Texture::IDType::Null();
		}

		return add(CreateFromData(size, data, format, desc, false));
	}

	Texture::IDType CTexture_GL4::create(const BCnData& bcnData)
	{
		if (not bcnData)
		{
			return Texture::IDType::Null();
		}

		const TextureFormat& format = bcnData.format;

		if (not isCompressedFormatSupported(format))
		{
			LOG_FAIL(U"❌ CTexture_GL4::create(): the GL context cannot sample {}"_fmt(format.name()));
			return Texture::IDType::Null();
		}

		const uint32 mipLevels = static_cast<uint32>(bcnData.textures.size());
		auto texture = MakeTexture(bcnData.size, mipLevels, format,
			((1 < mipLevels) ? TextureDesc{ TextureDesc::Mipmap } : TextureDesc{ TextureDesc::NoMipmap }), false);

		for (uint32 level = 0; level < mipLevels; ++level)
		{
			const Size mipSize = MipSize(bcnData.size, level);
			const size_t imageSize = (static_cast<size_t>(format.bytesPerRow(mipSize.x)) * ((mipSize.y + 3) / 4));
			const Blob& blob = bcnData.textures[level];

			if (blob.size() < imageSize)
			{
				::glBindTexture(GL_TEXTURE_2D, 0);
				return Texture::IDType::Null();
			}

			::glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), static_cast<GLenum>(format.GLInternalFormat()),
				mipSize.x, mipSize.y, 0, static_cast<GLsizei>(imageSize), blob.data());
		}

		::glBindTexture(GL_TEXTURE_2D, 0);
		return add(std::move(texture));
	}

	Texture::IDType CTexture_GL4::createDynamic(const Size& size, const std::span<const Byte> data, const TextureFormat& format, const TextureDesc desc)
	{
		if ((size.x <= 0) || (size.y <= 0) || (not IsUncompressedFormat(format))
			|| ((not data.empty()) && (data.size_bytes() < (static_cast<size_t>(format.bytesPerRow(size.x)) * size.y))))
		{
			return Texture::IDType::Null();
		}

		return add(CreateFromData(size, data, format, desc, true));
	}

	void CTexture_GL4::release(const Texture::IDType handleID)
	{
		m_textures.erase(handleID);
	}

	Size CTexture_GL4::getSize(const Texture::IDType handleID)
	{
		return m_textures[handleID]->size;
	}

	uint32 CTexture_GL4::getMipLevels(const Texture::IDType handleID)
	{
		return m_textures[handleID]->mipLevels;
	}

	TextureDesc CTexture_GL4::getDesc(const Texture::IDType handleID)
	{
		return m_textures[handleID]->desc;
	}

	TextureFormat CTexture_GL4::getFormat(const Texture::IDType handleID)
	{
		return m_textures[handleID]->format;
	}

	bool CTexture_GL4::hasDepth(const Texture::IDType)
	{
		return false;
	}

	bool CTexture_GL4::fill(const Texture::IDType handleID, const ColorF& color, const bool)
	{
		GLTexture& texture = *m_textures[handleID];

		if (not texture.dynamic)
		{
			return false;
		}

		FillRegionWithColor(texture, Rect{ texture.size }, color);
		return true;
	}

	bool CTexture_GL4::fill(const Texture::IDType handleID, const std::span<const Byte> src, const uint32 srcBytesPerRow, const bool)
	{
		GLTexture& texture = *m_textures[handleID];

		if (not texture.dynamic)
		{
			return false;
		}

		return UploadRows(texture, Rect{ texture.size }, src, srcBytesPerRow);
	}

	bool CTexture_GL4::fillRegion(const Texture::IDType handleID, const ColorF& color, const Rect& rect)
	{
		GLTexture& texture = *m_textures[handleID];

		if (not IsValidRegion(texture, rect))
		{
			return false;
		}

		FillRegionWithColor(texture, rect, color);
		return true;
	}

	bool CTexture_GL4::fillRegion(const Texture::IDType handleID, const std::span<const Byte> src, const uint32 srcBytesPerRow, const Rect& rect, const bool)
	{
		GLTexture& texture = *m_textures[handleID];

		if (not IsValidRegion(texture, rect))
		{
			return false;
		}

		return UploadRows(texture, rect, src, srcBytesPerRow);
	}

	void CTexture_GL4::generateMips(const Texture::IDType handleID)
	{
		const GLTexture& texture = *m_textures[handleID];

		if (texture.mipLevels <= 1)
		{
			return;
		}

		::glBindTexture(GL_TEXTURE_2D, texture.texture);
		::glGenerateMipmap(GL_TEXTURE_2D);
		::glBindTexture(GL_TEXTURE_2D, 0);
	}

	GLuint CTexture_GL4::getGLTexture(const Texture::IDType handleID)
	{
		return m_textures[handleID]->texture;
	}

	Texture::IDType CTexture_GL4::add(std::unique_ptr<GLTexture> texture)
	{
		const String info = U"(type: {}, size: {}, mipLevels: {}, format: {})"_fmt(
			(texture->dynamic ? U"Dynamic" : U"Default"), texture->size, texture->mipLevels, texture->format.name());
		return m_textures.add(std::move(texture), info);
	}

	bool CTexture_GL4::isCompressedFormatSupported(const TextureFormat& format) const noexcept
	{
		switch (format.value())
		{
		case TexturePixelFormat::BC1_RGBA_Unorm:
		case TexturePixelFormat::BC3_RGBA_Unorm:
			return m_s3tcSupported;
		case TexturePixelFormat::BC1_RGBA_Unorm_SRGB:
		case TexturePixelFormat::BC3_RGBA_Unorm_SRGB:
			return m_s3tcSRGBSupported;
		case TexturePixelFormat::BC4_R_Unorm:
		case TexturePixelFormat::BC5_RG_Unorm:
			return true;
		case TexturePixelFormat::BC7_RGBA_Unorm:
		case TexturePixelFormat::BC7_RGBA_Unorm_SRGB:
			return m_bptcSupported;
		default:
			return false;
		}
	}
}
