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
# include <cstdlib>
# include <memory>

TEST_CASE("JPEG.roundTrip")
{
	const Color color{ 200, 100, 50 };
	const Image source{ Size{ 32, 16 }, color };

	const Blob jpeg = JPEGEncoder{}.encode(source, 95);
	REQUIRE_FALSE(jpeg.isEmpty());

	const Image decoded = JPEGDecoder{}.decode(std::make_unique<MemoryViewReader>(jpeg.data(), jpeg.size()), U"", PremultiplyAlpha::No);
	REQUIRE(decoded.size() == source.size());

	// JPEG is lossy: a flat colour comes back within a small tolerance.
	const Color center = decoded[8][16];
	CHECK(std::abs(center.r - color.r) <= 4);
	CHECK(std::abs(center.g - color.g) <= 4);
	CHECK(std::abs(center.b - color.b) <= 4);
}
