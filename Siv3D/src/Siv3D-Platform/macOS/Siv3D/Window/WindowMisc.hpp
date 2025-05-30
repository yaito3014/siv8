//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2025 Ryo Suzuki
//	Copyright (c) 2016-2025 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/Common.hpp>

struct GLFWwindow;

namespace s3d::WindowMisc
{
	[[nodiscard]]
	int32 GetTitleBarHeight(GLFWwindow* glfwWindow);
}
