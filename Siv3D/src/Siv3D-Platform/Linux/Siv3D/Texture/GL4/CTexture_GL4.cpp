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
# include <Siv3D/Image.hpp>
# include <Siv3D/2DShapes.hpp>
# include <Siv3D/EngineLog.hpp>

namespace s3d
{
	namespace
	{
		// Phase 1 uploads everything as RGBA8 (the engine premultiplies alpha on
		// image load); other formats are TODO(linux).
		[[nodiscard]]
		static GLuint CreateGLTexture(const Size& size, const void* data, const bool dynamic)
		{
			GLuint texture = 0;
			::glGenTextures(1, &texture);
			::glBindTexture(GL_TEXTURE_2D, texture);
			::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			::glBindTexture(GL_TEXTURE_2D, 0);
			static_cast<void>(dynamic);
			return texture;
		}
	}

	CTexture_GL4::~CTexture_GL4()
	{
		LOG_SCOPED_DEBUG("CTexture_GL4::~CTexture_GL4()");
	}

	Texture::IDType CTexture_GL4::create(std::unique_ptr<IReader> reader, const FilePathView, const TextureDesc desc)
	{
		if ((not reader) || (not reader->isOpen()))
		{
			return Texture::IDType::Null();
		}

		const Image image{ std::move(reader) };
		return create(image, {}, desc);
	}

	Texture::IDType CTexture_GL4::create(const Image& image, const Array<Image>&, const TextureDesc desc)
	{
		if (not image)
		{
			return Texture::IDType::Null();
		}

		return create(image.size(), std::as_bytes(std::span{ image.data(), image.num_pixels() }), TextureFormat::R8G8B8A8_Unorm, desc);
	}

	Texture::IDType CTexture_GL4::create(const Size& size, const std::span<const Byte> data, const TextureFormat& format, const TextureDesc desc)
	{
		if ((size.x <= 0) || (size.y <= 0))
		{
			return Texture::IDType::Null();
		}

		auto glTexture = std::make_unique<GLTexture>();
		glTexture->texture	= CreateGLTexture(size, (data.empty() ? nullptr : data.data()), false);
		glTexture->size		= size;
		glTexture->desc		= desc;
		glTexture->format	= format;

		return m_textures.add(std::move(glTexture));
	}

	Texture::IDType CTexture_GL4::createDynamic(const Size& size, const std::span<const Byte> data, const TextureFormat& format, const TextureDesc desc)
	{
		if ((size.x <= 0) || (size.y <= 0))
		{
			return Texture::IDType::Null();
		}

		auto glTexture = std::make_unique<GLTexture>();
		glTexture->texture	= CreateGLTexture(size, (data.empty() ? nullptr : data.data()), true);
		glTexture->size		= size;
		glTexture->desc		= desc;
		glTexture->format	= format;

		return m_textures.add(std::move(glTexture));
	}

	void CTexture_GL4::release(const Texture::IDType handleID)
	{
		m_textures.erase(handleID);
	}

	Size CTexture_GL4::getSize(const Texture::IDType handleID)
	{
		if (const GLTexture* p = m_textures[handleID])
		{
			return p->size;
		}

		return{ 0, 0 };
	}

	TextureDesc CTexture_GL4::getDesc(const Texture::IDType handleID)
	{
		if (const GLTexture* p = m_textures[handleID])
		{
			return p->desc;
		}

		return{};
	}

	TextureFormat CTexture_GL4::getFormat(const Texture::IDType handleID)
	{
		if (const GLTexture* p = m_textures[handleID])
		{
			return p->format;
		}

		return{};
	}

	bool CTexture_GL4::fill(const Texture::IDType handleID, const std::span<const Byte> src, const uint32, const bool)
	{
		const GLTexture* p = m_textures[handleID];
		if ((not p) || src.empty())
		{
			return false;
		}

		::glBindTexture(GL_TEXTURE_2D, p->texture);
		::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		::glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, p->size.x, p->size.y, GL_RGBA, GL_UNSIGNED_BYTE, src.data());
		::glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	bool CTexture_GL4::fillRegion(const Texture::IDType handleID, const std::span<const Byte> src, const uint32, const Rect& rect, const bool)
	{
		const GLTexture* p = m_textures[handleID];
		if ((not p) || src.empty())
		{
			return false;
		}

		::glBindTexture(GL_TEXTURE_2D, p->texture);
		::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		::glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.w, rect.h, GL_RGBA, GL_UNSIGNED_BYTE, src.data());
		::glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	GLuint CTexture_GL4::getGLTexture(const Texture::IDType handleID)
	{
		if (const GLTexture* p = m_textures[handleID])
		{
			return p->texture;
		}

		return 0;
	}
}
