//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

// Entry point for the standalone Siv3D-Test executable. The engine bootstrap
// (Siv3DMain) calls Main(); here it just runs the doctest/nanobench suite.
//
// This lives in a subdirectory so the engine's recursive Test/*.cpp glob picks
// it up for Siv3D-Test, while the App's non-recursive ../Test/*.cpp glob does
// not (the App provides its own Main() in App/Main.cpp).

void RunTest();

void Main()
{
	RunTest();
}
