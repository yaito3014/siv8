//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <string>
# include <Siv3D/Common.hpp>
# include <Siv3D/Blob.hpp>
# include <Siv3D/Graphics.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// GL has a single global uniform-block binding namespace, whereas HLSL/the
	// engine use separate VS and PS slot spaces (VS b0, PS b0, PS b1). Map each
	// (stage, slot) to a distinct GL binding point: VS slot s -> s, PS slot s ->
	// PSUniformBlockBindingBase + s. Both the shader programs (via
	// glUniformBlockBinding) and CShader_GL4::setConstantBuffer*() use this map.
	inline constexpr GLuint VSUniformBlockBindingBase = 0;

	inline constexpr GLuint PSUniformBlockBindingBase = Graphics::ConstantBufferSlotCount;

	// Compile a single stage as a separable program object (GL 4.1 core
	// ARB_separate_shader_objects), so a VS and PS can be mixed independently
	// through a program pipeline — matching the per-stage ISiv3DShader interface.
	// Returns 0 on failure (compile/link error logged).
	[[nodiscard]]
	GLuint CreateSeparableShaderProgram(GLenum shaderType, const std::string& source);

	// Binds every active uniform block of a single-stage program to its GL binding
	// point. The engine's blocks (VSConstants2D, PSConstants2D, PSEffectConstants2D)
	// are matched by name. Any other block takes its stage-local slot from
	// `layout(binding = N)` (GLSL 4.20 or ARB_shading_language_420pack), like an
	// HLSL register(bN), and is moved to `stageBindingBase + N`.
	void AssignUniformBlockBindings(GLuint program, GLuint stageBindingBase);

	class GL4VertexShader
	{
	public:

		struct Null {};

		GL4VertexShader() = default;

		explicit GL4VertexShader(Null);

		explicit GL4VertexShader(const std::string& source);

		~GL4VertexShader();

		[[nodiscard]]
		bool isInitialized() const noexcept;

		[[nodiscard]]
		GLuint getProgram() const noexcept;

		[[nodiscard]]
		const Blob& getBytecode() const noexcept;

	private:

		Blob m_bytecode;

		GLuint m_program = 0;

		bool m_initialized = false;
	};

	class GL4PixelShader
	{
	public:

		struct Null {};

		GL4PixelShader() = default;

		explicit GL4PixelShader(Null);

		explicit GL4PixelShader(const std::string& source);

		~GL4PixelShader();

		[[nodiscard]]
		bool isInitialized() const noexcept;

		[[nodiscard]]
		GLuint getProgram() const noexcept;

		[[nodiscard]]
		const Blob& getBytecode() const noexcept;

	private:

		Blob m_bytecode;

		GLuint m_program = 0;

		bool m_initialized = false;
	};
}
