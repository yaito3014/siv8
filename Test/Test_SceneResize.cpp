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

namespace
{
	[[nodiscard]]
	Image CaptureFrame(const std::function<void()>& draw)
	{
		REQUIRE(System::Update());
		draw();
		ScreenCapture::RequestCurrentFrame();
		REQUIRE(System::Update());
		REQUIRE(ScreenCapture::HasNewFrame());
		return ScreenCapture::GetFrame();
	}
}

TEST_CASE("Scene.ResizeModes")
{
	const ResizeMode previousMode = Scene::GetResizeMode();
	const ScopeExit restore{ [&] { Scene::SetResizeMode(previousMode); } };

	Scene::SetResizeMode(ResizeMode::Virtual);
	const Size virtualSize = Scene::Size();

	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(320, 200);
	CHECK(Scene::Size() == Size{ 320, 200 });

	// The capture is the scene, not the window, and scene coordinates span the whole buffer.
	{
		const Image image = CaptureFrame([]
		{
			Rect{ 0, 0, 320, 200 }.draw(Palette::Red);
			Rect{ 300, 180, 20, 20 }.draw(Palette::Blue);
		});

		CHECK(image.size() == Size{ 320, 200 });
		CHECK(image[10][10] == Palette::Red);
		CHECK(image[190][310] == Palette::Blue);
		CHECK(image[199][319] == Palette::Blue);
	}

	CHECK(Scene::Size() == Size{ 320, 200 });

	Scene::SetResizeMode(ResizeMode::Virtual);
	CHECK(Scene::Size() == virtualSize);

	{
		const Image image = CaptureFrame([] { Rect{ 0, 0, 10, 10 }.draw(Palette::Lime); });
		CHECK(image.size() == virtualSize);
		CHECK(image[5][5] == Palette::Lime);
	}
}

TEST_CASE("Scene.Resize.BackgroundFillsTheNewBuffer")
{
	const ResizeMode previousMode = Scene::GetResizeMode();
	const ColorF previousBackground = Scene::GetBackground();
	const ScopeExit restore{ [&]
	{
		Scene::SetBackground(previousBackground);
		Scene::SetResizeMode(previousMode);
	} };

	Scene::SetBackground(Palette::Black);
	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(64, 48);

	const Image image = CaptureFrame([] {});
	CHECK(image.size() == Size{ 64, 48 });
	CHECK(std::all_of(image.begin(), image.end(), [](const Color& c) { return (c == Palette::Black); }));
}
