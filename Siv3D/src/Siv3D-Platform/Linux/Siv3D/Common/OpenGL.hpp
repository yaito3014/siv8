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
# include <string_view>
// glad provides the GL 4.1 function pointers; tell GLFW not to pull in its own
// GL headers so the two don't collide.
# define GLFW_INCLUDE_NONE
# include <glad/glad.h>
# include <GLFW/glfw3.h>

// The glad build is GL 4.1 core without extensions, so the extension enums used
// at run time are declared here (values from the Khronos registry).
# ifndef GL_TEXTURE_MAX_ANISOTROPY
#	define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
# endif
# ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY
#	define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
# endif

namespace s3d
{
	/// @brief Returns whether the current GL context advertises the extension.
	/// @param name Extension name, e.g. "GL_EXT_texture_compression_s3tc"
	[[nodiscard]]
	inline bool HasGLExtension(const std::string_view name)
	{
		GLint count = 0;
		::glGetIntegerv(GL_NUM_EXTENSIONS, &count);

		for (GLint i = 0; i < count; ++i)
		{
			if (const auto* extension = reinterpret_cast<const char*>(::glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))))
			{
				if (name == extension)
				{
					return true;
				}
			}
		}

		return false;
	}
}
