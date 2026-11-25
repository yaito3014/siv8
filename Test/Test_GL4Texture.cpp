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

# include "Siv3DTest.hpp"

# if SIV3D_PLATFORM(LINUX)

# include <cstring>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/Texture/GL4/CTexture_GL4.hpp>
# include <Siv3D/Texture/TextureUtility.hpp>

namespace
{
	[[nodiscard]]
	CTexture_GL4& TextureBackend()
	{
		return *static_cast<CTexture_GL4*>(SIV3D_ENGINE(Texture));
	}

	// Releases an internal texture handle at the end of a test.
	struct ScopedTextureID
	{
		Texture::IDType id;

		~ScopedTextureID()
		{
			if (not id.isNull())
			{
				TextureBackend().release(id);
			}
		}
	};

	[[nodiscard]]
	Array<Byte> ReadLevel(const Texture::IDType id, const uint32 level, const GLenum glFormat, const GLenum glType, const size_t byteCount)
	{
		Array<Byte> pixels(byteCount);
		::glBindTexture(GL_TEXTURE_2D, TextureBackend().getGLTexture(id));
		::glPixelStorei(GL_PACK_ALIGNMENT, 4);
		::glGetTexImage(GL_TEXTURE_2D, static_cast<GLint>(level), glFormat, glType, pixels.data());
		::glBindTexture(GL_TEXTURE_2D, 0);
		return pixels;
	}

	[[nodiscard]]
	Array<Byte> ReadLevel(const Texture::IDType id, const uint32 level, const TextureFormat& format, const Size& size)
	{
		return ReadLevel(id, level, static_cast<GLenum>(format.GLFormat()), static_cast<GLenum>(format.GLType()),
			(static_cast<size_t>(format.bytesPerRow(size.x)) * size.y));
	}

	[[nodiscard]]
	Array<Color> ReadRGBA8(const Texture::IDType id, const uint32 level, const Size& size)
	{
		const Array<Byte> bytes = ReadLevel(id, level, GL_RGBA, GL_UNSIGNED_BYTE, (static_cast<size_t>(size.x) * size.y * sizeof(Color)));
		Array<Color> pixels(static_cast<size_t>(size.x) * size.y);
		std::memcpy(pixels.data(), bytes.data(), bytes.size());
		return pixels;
	}

	template <class Element>
	void Store(Array<Byte>& bytes, const size_t offset, const Element value)
	{
		std::memcpy((bytes.data() + offset), &value, sizeof(Element));
	}

	// Pixel data whose every value survives a GL round trip unchanged: arbitrary bits for
	// integer formats, finite values for the floating-point ones.
	[[nodiscard]]
	Array<Byte> MakePixels(const TextureFormat& format, const Size& size)
	{
		const uint32 bytesPerRow = format.bytesPerRow(size.x);
		Array<Byte> bytes((static_cast<size_t>(bytesPerRow) * size.y), Byte{ 0 });
		const uint32 pixelSize = format.pixelSize();
		const uint32 componentCount = format.channelCount();
		uint32 counter = 1;

		for (int32 y = 0; y < size.y; ++y)
		{
			for (int32 x = 0; x < size.x; ++x)
			{
				const size_t pixelOffset = ((static_cast<size_t>(y) * bytesPerRow) + (static_cast<size_t>(x) * pixelSize));

				switch (static_cast<GLenum>(format.GLType()))
				{
				case GL_FLOAT:
					for (uint32 c = 0; c < componentCount; ++c)
					{
						Store(bytes, (pixelOffset + (c * 4)), (static_cast<float>(counter++) * 0.25f));
					}
					break;
				case GL_HALF_FLOAT:
					for (uint32 c = 0; c < componentCount; ++c)
					{
						Store(bytes, (pixelOffset + (c * 2)), HalfFloat{ static_cast<float>(counter++) * 0.25f }.getBits());
					}
					break;
				case GL_UNSIGNED_INT_10F_11F_11F_REV:
					// 1.0 in each channel: 11-bit and 10-bit floats with a biased exponent of 15.
					Store(bytes, pixelOffset, static_cast<uint32>((0x3C0u) | (0x3C0u << 11) | (0x1E0u << 22)));
					break;
				default:
					for (uint32 i = 0; i < pixelSize; ++i)
					{
						bytes[pixelOffset + i] = static_cast<Byte>((counter++ * 37) & 0xFF);
					}
					break;
				}
			}
		}

		return bytes;
	}

	void CheckRowsEqual(const Array<Byte>& expected, const Array<Byte>& actual, const TextureFormat& format, const Size& size)
	{
		const uint32 bytesPerRow = format.bytesPerRow(size.x);
		const size_t usedBytesPerRow = (static_cast<size_t>(size.x) * format.pixelSize());

		for (int32 y = 0; y < size.y; ++y)
		{
			INFO(y);
			CHECK(std::memcmp((expected.data() + (static_cast<size_t>(y) * bytesPerRow)),
				(actual.data() + (static_cast<size_t>(y) * bytesPerRow)), usedBytesPerRow) == 0);
		}
	}
}

TEST_CASE("GL4Texture.UncompressedFormatsRoundTrip")
{
	constexpr Size size{ 3, 2 };

	for (const TextureFormat format : { TextureFormat::R8_Unorm, TextureFormat::R8G8_Unorm, TextureFormat::R16_Float,
		TextureFormat::R8G8B8A8_Unorm, TextureFormat::R8G8B8A8_Unorm_SRGB, TextureFormat::R16G16_Unorm,
		TextureFormat::R16G16_Float, TextureFormat::R32_Float, TextureFormat::R10G10B10A2_Unorm,
		TextureFormat::R11G11B10_UFloat, TextureFormat::R16G16B16A16_Float, TextureFormat::R32G32_Float,
		TextureFormat::R32G32B32A32_Float })
	{
		INFO(format.name());
		const Array<Byte> pixels = MakePixels(format, size);
		const ScopedTextureID texture{ TextureBackend().create(size, pixels, format, TextureDesc::NoMipmap) };
		REQUIRE(not texture.id.isNull());
		CHECK(TextureBackend().getFormat(texture.id) == format);
		CHECK(TextureBackend().getMipLevels(texture.id) == 1);
		CheckRowsEqual(pixels, ReadLevel(texture.id, 0, format, size), format, size);
	}
}

TEST_CASE("GL4Texture.CreateRejectsShortData")
{
	const Array<Byte> pixels(15);
	CHECK(TextureBackend().create(Size{ 2, 2 }, pixels, TextureFormat::R8G8B8A8_Unorm, TextureDesc::NoMipmap).isNull());
	CHECK(TextureBackend().createDynamic(Size{ 2, 2 }, pixels, TextureFormat::R8G8B8A8_Unorm, TextureDesc::NoMipmap).isNull());
	CHECK(TextureBackend().create(Size{ 4, 4 }, {}, TextureFormat::BC1_RGBA_Unorm, TextureDesc::NoMipmap).isNull());
}

TEST_CASE("GL4Texture.SuppliedMipmaps")
{
	const Image image{ 8, Palette::Red };
	const Array<Image> mipmaps = { Image{ 4, Palette::Lime }, Image{ 2, Palette::Blue }, Image{ 1, Palette::White } };

	const ScopedTextureID texture{ TextureBackend().create(image, mipmaps, TextureDesc::Mipmap) };
	REQUIRE(not texture.id.isNull());
	CHECK(TextureBackend().getMipLevels(texture.id) == 4);
	CHECK(ReadRGBA8(texture.id, 0, Size{ 8, 8 }).all([](const Color& c) { return (c == Palette::Red); }));
	CHECK(ReadRGBA8(texture.id, 1, Size{ 4, 4 }).all([](const Color& c) { return (c == Palette::Lime); }));
	CHECK(ReadRGBA8(texture.id, 2, Size{ 2, 2 }).all([](const Color& c) { return (c == Palette::Blue); }));
	CHECK(ReadRGBA8(texture.id, 3, Size{ 1, 1 }).front() == Palette::White);

	const ScopedTextureID noMipmap{ TextureBackend().create(image, mipmaps, TextureDesc::NoMipmap) };
	CHECK(TextureBackend().getMipLevels(noMipmap.id) == 1);

	const ScopedTextureID srgb{ TextureBackend().create(image, {}, TextureDesc::NoMipmapSRGB) };
	CHECK(TextureBackend().getFormat(srgb.id) == TextureFormat::R8G8B8A8_Unorm_SRGB);
}

TEST_CASE("GL4Texture.GeneratedMipmaps")
{
	const Image image{ Size{ 16, 4 }, Color{ 10, 20, 30, 40 } };
	const ScopedTextureID texture{ TextureBackend().create(image.size(), std::as_bytes(std::span{ image }), TextureFormat::R8G8B8A8_Unorm, TextureDesc::Mipmap) };
	REQUIRE(not texture.id.isNull());
	REQUIRE(TextureBackend().getMipLevels(texture.id) == 5);
	CHECK(ReadRGBA8(texture.id, 2, Size{ 4, 1 }).all([](const Color& c) { return (c == Color{ 10, 20, 30, 40 }); }));
	CHECK(ReadRGBA8(texture.id, 4, Size{ 1, 1 }).front() == Color{ 10, 20, 30, 40 });
}

TEST_CASE("GL4Texture.DynamicFill")
{
	constexpr Size size{ 5, 3 };
	const TextureFormat format = TextureFormat::R8G8B8A8_Unorm;
	const ScopedTextureID texture{ TextureBackend().createDynamic(size, GenerateInitialColorBuffer(size, Palette::Black, format), format, TextureDesc::NoMipmap) };
	REQUIRE(not texture.id.isNull());

	CHECK(TextureBackend().fill(texture.id, ColorF{ Palette::Red }, true));
	CHECK(ReadRGBA8(texture.id, 0, size).all([](const Color& c) { return (c == Palette::Red); }));

	CHECK(TextureBackend().fillRegion(texture.id, ColorF{ Palette::Blue }, Rect{ 1, 1, 2, 2 }));
	{
		const Array<Color> pixels = ReadRGBA8(texture.id, 0, size);
		CHECK(pixels[(1 * size.x) + 1] == Palette::Blue);
		CHECK(pixels[(2 * size.x) + 2] == Palette::Blue);
		CHECK(pixels[(1 * size.x) + 3] == Palette::Red);
		CHECK(pixels[0] == Palette::Red);
	}

	// Source rows padded by whole pixels (GL strides) and by a partial pixel (repacked).
	for (const uint32 padding : { 4u, 2u })
	{
		INFO(padding);
		const uint32 srcBytesPerRow = ((size.x * 4) + padding);
		Array<Byte> src((static_cast<size_t>(srcBytesPerRow) * size.y), Byte{ 0xEE });

		for (int32 y = 0; y < size.y; ++y)
		{
			for (int32 x = 0; x < size.x; ++x)
			{
				const Color color{ static_cast<uint8>(x * 40), static_cast<uint8>(y * 80), static_cast<uint8>(padding), 255 };
				std::memcpy((src.data() + (y * srcBytesPerRow) + (x * 4)), &color, sizeof(Color));
			}
		}

		CHECK(TextureBackend().fill(texture.id, src, srcBytesPerRow, true));
		const Array<Color> pixels = ReadRGBA8(texture.id, 0, size);
		CHECK(pixels[(2 * size.x) + 4] == Color{ 160, 160, static_cast<uint8>(padding), 255 });
		CHECK(pixels[0] == Color{ 0, 0, static_cast<uint8>(padding), 255 });

		const Rect rect{ 3, 1, 2, 2 };
		CHECK(TextureBackend().fillRegion(texture.id, src, srcBytesPerRow, rect, true));
		CHECK(ReadRGBA8(texture.id, 0, size)[(2 * size.x) + 4] == Color{ 40, 80, static_cast<uint8>(padding), 255 });
	}

	CHECK_FALSE(TextureBackend().fill(texture.id, Array<Byte>(8), (size.x * 4), true));
	CHECK_FALSE(TextureBackend().fillRegion(texture.id, ColorF{ Palette::Blue }, Rect{ 4, 0, 2, 1 }));
	CHECK_FALSE(TextureBackend().fillRegion(texture.id, Array<Byte>(64), 3, Rect{ 0, 0, 2, 2 }, true));
}

TEST_CASE("GL4Texture.StaticTextureRejectsFill")
{
	const Image image{ 4, Palette::Red };
	const ScopedTextureID texture{ TextureBackend().create(image, {}, TextureDesc::NoMipmap) };
	REQUIRE(not texture.id.isNull());
	CHECK_FALSE(TextureBackend().fill(texture.id, ColorF{ Palette::Blue }, true));
	CHECK_FALSE(TextureBackend().fill(texture.id, std::as_bytes(std::span{ image }), image.bytesPerRow(), true));
	CHECK_FALSE(TextureBackend().fillRegion(texture.id, ColorF{ Palette::Blue }, Rect{ 0, 0, 1, 1 }));
	CHECK(ReadRGBA8(texture.id, 0, image.size()).all([](const Color& c) { return (c == Palette::Red); }));
}

TEST_CASE("GL4Texture.GenerateMipsAfterFill")
{
	constexpr Size size{ 8, 8 };
	const TextureFormat format = TextureFormat::R8G8B8A8_Unorm;
	const ScopedTextureID texture{ TextureBackend().createDynamic(size, GenerateInitialColorBuffer(size, Palette::Black, format), format, TextureDesc::Mipmap) };
	REQUIRE(TextureBackend().getMipLevels(texture.id) == 4);

	REQUIRE(TextureBackend().fill(texture.id, ColorF{ Palette::Lime }, true));
	TextureBackend().generateMips(texture.id);
	CHECK(ReadRGBA8(texture.id, 3, Size{ 1, 1 }).front() == Palette::Lime);
}

TEST_CASE("GL4Texture.BC4")
{
	// One 4x4 block: endpoints 255 and 0, every index 0 -> red channel 255 everywhere.
	BCnData data;
	data.size = Size{ 4, 4 };
	data.format = TextureFormat::BC4_R_Unorm;
	const std::array<uint8, 8> block{ 255, 0, 0, 0, 0, 0, 0, 0 };
	data.textures.push_back(Blob{ block.data(), block.size() });

	const ScopedTextureID texture{ TextureBackend().create(data) };
	REQUIRE(not texture.id.isNull());
	CHECK(TextureBackend().getFormat(texture.id) == TextureFormat::BC4_R_Unorm);
	CHECK(ReadLevel(texture.id, 0, GL_RED, GL_UNSIGNED_BYTE, 16).all([](const Byte b) { return (b == Byte{ 255 }); }));

	BCnData truncated = data;
	truncated.textures.front() = Blob{ block.data(), 4 };
	CHECK(TextureBackend().create(truncated).isNull());
}

TEST_CASE("GL4Texture.BC1")
{
	if (not HasGLExtension("GL_EXT_texture_compression_s3tc"))
	{
		SKIP("GL_EXT_texture_compression_s3tc is not available");
	}

	// One 4x4 block: color0 = color1 = pure red (RGB565 0xF800), every index 0.
	BCnData data;
	data.size = Size{ 4, 4 };
	data.format = TextureFormat::BC1_RGBA_Unorm;
	const std::array<uint8, 8> block{ 0x00, 0xF8, 0x00, 0xF8, 0, 0, 0, 0 };
	data.textures.push_back(Blob{ block.data(), block.size() });

	const ScopedTextureID texture{ TextureBackend().create(data) };
	REQUIRE(not texture.id.isNull());
	CHECK(ReadRGBA8(texture.id, 0, Size{ 4, 4 }).all([](const Color& c) { return (c == Color{ 255, 0, 0, 255 }); }));
}

TEST_CASE("GL4Texture.NullTexture")
{
	const Texture::IDType nullID = Texture::IDType::Null();
	CHECK(TextureBackend().getGLTexture(nullID) != 0);
	CHECK(TextureBackend().getSize(nullID) == Size{ 16, 16 });
	CHECK(TextureBackend().getMipLevels(nullID) == 5);
	CHECK(ReadRGBA8(nullID, 0, Size{ 16, 16 }).all([](const Color& c) { return (c == Palette::Yellow); }));
}

# endif
