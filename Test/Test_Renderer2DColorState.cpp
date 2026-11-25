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

TEST_CASE("Renderer2D.ColorMulAndAddApplyPerDraw")
{
	const ColorF previousBackground = Scene::GetBackground();
	const ScopeExit restore{ [&] { Scene::SetBackground(previousBackground); } };
	Scene::SetBackground(Palette::Black);

	REQUIRE(System::Update());

	Rect{ 0, 0, 20, 20 }.draw(Palette::White);
	{
		const ScopedColorMul2D colorMul{ Palette::Red };
		Rect{ 40, 0, 20, 20 }.draw(Palette::White);
	}
	{
		const ScopedColorAdd2D colorAdd{ ColorF{ 0.0, 0.0, 1.0 } };
		Rect{ 80, 0, 20, 20 }.draw(Palette::Black);
	}
	Rect{ 120, 0, 20, 20 }.draw(Palette::White);

	ScreenCapture::RequestCurrentFrame();
	REQUIRE(System::Update());
	REQUIRE(ScreenCapture::HasNewFrame());
	const Image image = ScreenCapture::GetFrame();

	CHECK(image[10][10] == Palette::White);
	CHECK(image[10][50] == Palette::Red);
	CHECK(image[10][90] == Palette::Blue);
	CHECK(image[10][130] == Palette::White);
}
