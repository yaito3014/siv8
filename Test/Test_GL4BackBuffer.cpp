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

# include <algorithm>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/Renderer/IRenderer.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace
{
	// ScreenCapture reads the scene buffer, so these tests read what the renderer
	// put into the window's back buffer, before the frame is swapped.
	[[nodiscard]]
	Image ReadBackBuffer()
	{
		const Size size = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		Image image{ size };

		::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		::glReadBuffer(GL_BACK);
		::glPixelStorei(GL_PACK_ALIGNMENT, 4);
		::glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

		Color* const pixels = image.data();

		for (int32 y = 0; y < (size.y / 2); ++y)
		{
			std::swap_ranges((pixels + (static_cast<size_t>(y) * size.x)), (pixels + (static_cast<size_t>(y + 1) * size.x)),
				(pixels + (static_cast<size_t>(size.y - 1 - y) * size.x)));
		}

		return image;
	}

	template <class Draw>
	[[nodiscard]]
	Image RenderToWindow(Draw&& draw)
	{
		REQUIRE(System::Update());
		draw();

		while (::glGetError() != GL_NO_ERROR) {}
		SIV3D_ENGINE(Renderer)->flush();
		CHECK(::glGetError() == GL_NO_ERROR);

		return ReadBackBuffer();
	}

	[[nodiscard]]
	bool SameRGB(const Color a, const Color b) noexcept
	{
		return ((a.r == b.r) && (a.g == b.g) && (a.b == b.b));
	}

	struct SceneStateScope
	{
		ResizeMode resizeMode = Scene::GetResizeMode();
		ColorF background = Scene::GetBackground();
		ColorF letterbox = Scene::GetLetterbox();

		~SceneStateScope()
		{
			Scene::SetLetterbox(letterbox);
			Scene::SetBackground(background);
			Scene::SetResizeMode(resizeMode);
		}
	};
}

TEST_CASE("GL4BackBuffer.SameSizeScene")
{
	const SceneStateScope restore;
	Scene::SetResizeMode(ResizeMode::Actual);
	Scene::SetBackground(Palette::Black);

	const Size size = SIV3D_ENGINE(Window)->getState().frameBufferSize;
	REQUIRE(Scene::Size() == size);

	const Image image = RenderToWindow([&]
	{
		Rect{ 0, 0, 10, 10 }.draw(Palette::Blue);
		Rect{ (size.x - 10), (size.y - 10), 10, 10 }.draw(Palette::Red);
	});

	CHECK(SameRGB(image[5][5], Palette::Blue));
	CHECK(SameRGB(image[size.y - 5][size.x - 5], Palette::Red));
	CHECK(SameRGB(image[size.y / 2][size.x / 2], Palette::Black));
}

TEST_CASE("GL4BackBuffer.LetterboxedScene")
{
	const SceneStateScope restore;
	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(200, 100);
	Scene::SetBackground(Palette::Red);
	Scene::SetLetterbox(Palette::Lime);

	const Size windowSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
	const auto [scale, viewRect] = SIV3D_ENGINE(Renderer)->getLetterboxComposition();
	REQUIRE(1.0 < scale);
	REQUIRE(((0.0 < viewRect.x) || (0.0 < viewRect.y)));

	const Image image = RenderToWindow([] { Rect{ 0, 0, 20, 20 }.draw(Palette::Blue); });

	// The scene's top-left marker lands at the top-left of the letterbox rectangle,
	// scaled, and the bars outside it are the letterbox color.
	const Point marker = (viewRect.pos + Vec2{ (10 * scale), (10 * scale) }).asPoint();
	CHECK(SameRGB(image[marker.y][marker.x], Palette::Blue));
	CHECK(SameRGB(image[static_cast<int32>(viewRect.y + viewRect.h / 2)][static_cast<int32>(viewRect.x + viewRect.w / 2)], Palette::Red));

	if (0.0 < viewRect.y)
	{
		CHECK(SameRGB(image[static_cast<int32>(viewRect.y / 2)][windowSize.x / 2], Palette::Lime));
		CHECK(SameRGB(image[static_cast<int32>(viewRect.y + viewRect.h + ((windowSize.y - viewRect.y - viewRect.h) / 2))][windowSize.x / 2], Palette::Lime));
	}
	else
	{
		CHECK(SameRGB(image[windowSize.y / 2][static_cast<int32>(viewRect.x / 2)], Palette::Lime));
		CHECK(SameRGB(image[windowSize.y / 2][static_cast<int32>(viewRect.x + viewRect.w + ((windowSize.x - viewRect.x - viewRect.w) / 2))], Palette::Lime));
	}
}

# endif
