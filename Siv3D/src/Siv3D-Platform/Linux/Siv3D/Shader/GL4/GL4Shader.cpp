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

	void AssociateEngineUniformBlocks(const GLuint program)
	{
		struct BlockBinding
		{
			const char* name;
			GLuint binding;
		};

		// Names + bindings mirror the engine's cbuffer slots (see GL4Shader.hpp).
		const BlockBinding blocks[] = {
			{ "VSConstants2D",       (VSUniformBlockBindingBase + 0) },
			{ "PSConstants2D",       (PSUniformBlockBindingBase + 0) },
			{ "PSEffectConstants2D", (PSUniformBlockBindingBase + 1) },
		};

		for (const auto& block : blocks)
		{
			const GLuint index = ::glGetUniformBlockIndex(program, block.name);

			if (index != GL_INVALID_INDEX)
			{
				::glUniformBlockBinding(program, index, block.binding);
			}
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
			AssociateEngineUniformBlocks(m_program);
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
			AssociateEngineUniformBlocks(m_program);
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
