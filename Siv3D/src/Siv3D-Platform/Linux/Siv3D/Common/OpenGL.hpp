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

# pragma once
// glad provides the GL 4.1 function pointers; tell GLFW not to pull in its own
// GL headers so the two don't collide.
# define GLFW_INCLUDE_NONE
# include <glad/glad.h>
# include <GLFW/glfw3.h>
