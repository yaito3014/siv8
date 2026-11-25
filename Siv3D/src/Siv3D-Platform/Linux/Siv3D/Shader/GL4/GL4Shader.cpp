//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "GL4Shader.hpp"
# include <algorithm>
# include <iterator>
# include <string_view>
# include <Siv3D/EngineLog.hpp>

namespace s3d
{
	GLuint CreateSeparableShaderProgram(const GLenum shaderType, const std::string& source)
	{
		const char* const sourcePtr = source.c_str();

		// Creates a program, compiles `source` as `shaderType`, links it, and marks
		// it GL_PROGRAM_SEPARABLE — all in one call.
		const GLuint program = ::glCreateShaderProgramv(shaderType, 1, &sourcePtr);

		if (program == 0)
		{
			LOG_FAIL(U"❌ CreateSeparableShaderProgram(): glCreateShaderProgramv() returned 0");
			return 0;
		}

		GLint linked = GL_FALSE;
		::glGetProgramiv(program, GL_LINK_STATUS, &linked);

		if (linked != GL_TRUE)
		{
			GLint logLength = 0;
			::glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

			std::string log(static_cast<size_t>(logLength), '\0');
			if (0 < logLength)
			{
				::glGetProgramInfoLog(program, logLength, nullptr, log.data());
			}

			LOG_FAIL(fmt::format("❌ CreateSeparableShaderProgram(): shader compile/link failed: {}", log));

			::glDeleteProgram(program);
			return 0;
		}

		return program;
	}

	void AssignUniformBlockBindings(const GLuint program, const GLuint stageBindingBase)
	{
		struct BlockBinding
		{
			std::string_view name;
			GLuint binding;
		};

		// Names + bindings mirror the engine's cbuffer slots (see GL4Shader.hpp).
		constexpr BlockBinding EngineBlocks[] = {
			{ "VSConstants2D",       (VSUniformBlockBindingBase + 0) },
			{ "PSConstants2D",       (PSUniformBlockBindingBase + 0) },
			{ "PSEffectConstants2D", (PSUniformBlockBindingBase + 1) },
		};

		GLint blockCount = 0;
		::glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);

		for (GLuint index = 0; index < static_cast<GLuint>(blockCount); ++index)
		{
			GLint nameLength = 0;
			::glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);

			std::string name(static_cast<size_t>(Max(nameLength, 1)), '\0');
			GLsizei written = 0;
			::glGetActiveUniformBlockName(program, index, static_cast<GLsizei>(name.size()), &written, name.data());
			name.resize(static_cast<size_t>(written));

			const auto engineBlock = std::find_if(std::begin(EngineBlocks), std::end(EngineBlocks),
				[&](const BlockBinding& block) { return (block.name == name); });

			if (engineBlock != std::end(EngineBlocks))
			{
				::glUniformBlockBinding(program, index, engineBlock->binding);
				continue;
			}

			GLint slot = 0;
			::glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_BINDING, &slot);

			if ((slot < 0) || (Graphics::ConstantBufferSlotCount <= static_cast<uint32>(slot)))
			{
				LOG_FAIL(fmt::format("❌ AssignUniformBlockBindings(): uniform block `{}` uses binding {}, but the slot must be less than {}",
					name, slot, Graphics::ConstantBufferSlotCount));
				continue;
			}

			::glUniformBlockBinding(program, index, (stageBindingBase + static_cast<GLuint>(slot)));
		}
	}

	//
	//	GL4VertexShader
	//
	GL4VertexShader::GL4VertexShader(Null)
		: m_initialized{ true } {}

	GL4VertexShader::GL4VertexShader(const std::string& source)
		: m_bytecode{ source.data(), source.size() }
	{
		m_program = CreateSeparableShaderProgram(GL_VERTEX_SHADER, source);

		if (m_program)
		{
			AssignUniformBlockBindings(m_program, VSUniformBlockBindingBase);
			m_initialized = true;
		}
	}

	GL4VertexShader::~GL4VertexShader()
	{
		if (m_program)
		{
			::glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	bool GL4VertexShader::isInitialized() const noexcept
	{
		return m_initialized;
	}

	GLuint GL4VertexShader::getProgram() const noexcept
	{
		return m_program;
	}

	const Blob& GL4VertexShader::getBytecode() const noexcept
	{
		return m_bytecode;
	}

	//
	//	GL4PixelShader
	//
	GL4PixelShader::GL4PixelShader(Null)
		: m_initialized{ true } {}

	GL4PixelShader::GL4PixelShader(const std::string& source)
		: m_bytecode{ source.data(), source.size() }
	{
		m_program = CreateSeparableShaderProgram(GL_FRAGMENT_SHADER, source);

		if (m_program)
		{
			AssignUniformBlockBindings(m_program, PSUniformBlockBindingBase);
			m_initialized = true;
		}
	}

	GL4PixelShader::~GL4PixelShader()
	{
		if (m_program)
		{
			::glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	bool GL4PixelShader::isInitialized() const noexcept
	{
		return m_initialized;
	}

	GLuint GL4PixelShader::getProgram() const noexcept
	{
		return m_program;
	}

	const Blob& GL4PixelShader::getBytecode() const noexcept
	{
		return m_bytecode;
	}
}
