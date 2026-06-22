//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CRenderer2D_GL4.hpp"
# include <cstddef>
# include <Siv3D/Unicode.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/Texture/ITexture.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/Error/InternalEngineError.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Texture/GL4/CTexture_GL4.hpp>

namespace s3d
{
	namespace
	{
		// Shared VS: 2D affine + screen->clip packed into (u_t0, u_t1) like the
		// D3D11/Metal VSConstants2D; colors premultiplied.
		constexpr StringView VSCode =
UR"(#version 410 core
layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec2 i_tex;
layout(location = 2) in vec4 i_color;
uniform vec4 u_t0;
uniform vec4 u_t1;
uniform vec4 u_colorMul;
out vec4 v_color;
out vec2 v_uv;
void main()
{
	vec2 xy = vec2(u_t0.z, u_t0.w) + (i_pos.x * vec2(u_t0.x, u_t0.y)) + (i_pos.y * vec2(u_t1.x, u_t1.y));
	gl_Position = vec4(xy, u_t1.z, u_t1.w);
	vec4 c = (i_color * u_colorMul);
	v_color = vec4((c.rgb * c.a), c.a);
	v_uv = i_tex;
}
)";

		constexpr StringView ShapePSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
void main()
{
	o_color = v_color;
}
)";

		// The engine premultiplies alpha on image load, so the sampled texel is
		// already PMA. V is flipped (GL texture origin is bottom-left).
		constexpr StringView TexturePSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform sampler2D u_tex;
void main()
{
	vec4 t = texture(u_tex, vec2(v_uv.x, (1.0 - v_uv.y)));
	o_color = (t * v_color);
}
)";

		[[nodiscard]]
		static GLuint CompileShader(const GLenum type, const StringView code)
		{
			const std::string utf8 = Unicode::ToUTF8(code);
			const char* src = utf8.c_str();

			const GLuint shader = ::glCreateShader(type);
			::glShaderSource(shader, 1, &src, nullptr);
			::glCompileShader(shader);

			GLint status = GL_FALSE;
			::glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
			if (status != GL_TRUE)
			{
				char log[2048] = {};
				::glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
				::glDeleteShader(shader);
				throw InternalEngineError{ std::string{ "GL2D shader compile failed: " } + log };
			}

			return shader;
		}

		[[nodiscard]]
		static GLuint LinkProgram(const StringView psCode)
		{
			const GLuint vs = CompileShader(GL_VERTEX_SHADER, VSCode);
			const GLuint ps = CompileShader(GL_FRAGMENT_SHADER, psCode);

			const GLuint program = ::glCreateProgram();
			::glAttachShader(program, vs);
			::glAttachShader(program, ps);
			::glLinkProgram(program);

			GLint status = GL_FALSE;
			::glGetProgramiv(program, GL_LINK_STATUS, &status);
			if (status != GL_TRUE)
			{
				char log[2048] = {};
				::glGetProgramInfoLog(program, sizeof(log), nullptr, log);
				throw InternalEngineError{ std::string{ "GL2D program link failed: " } + log };
			}

			::glDeleteShader(vs);
			::glDeleteShader(ps);
			return program;
		}
	}

	CRenderer2D_GL4::~CRenderer2D_GL4()
	{
		LOG_SCOPED_DEBUG("CRenderer2D_GL4::~CRenderer2D_GL4()");

		if (m_textureProgram) { ::glDeleteProgram(m_textureProgram); }
		if (m_program) { ::glDeleteProgram(m_program); }
		if (m_ibo) { ::glDeleteBuffers(1, &m_ibo); }
		if (m_vbo) { ::glDeleteBuffers(1, &m_vbo); }
		if (m_vao) { ::glDeleteVertexArrays(1, &m_vao); }
	}

	void CRenderer2D_GL4::init()
	{
		LOG_SCOPED_DEBUG("CRenderer2D_GL4::init()");

		m_program = LinkProgram(ShapePSCode);
		m_locTransform0	= ::glGetUniformLocation(m_program, "u_t0");
		m_locTransform1	= ::glGetUniformLocation(m_program, "u_t1");
		m_locColorMul	= ::glGetUniformLocation(m_program, "u_colorMul");

		m_textureProgram = LinkProgram(TexturePSCode);
		m_texLocTransform0	= ::glGetUniformLocation(m_textureProgram, "u_t0");
		m_texLocTransform1	= ::glGetUniformLocation(m_textureProgram, "u_t1");
		m_texLocColorMul	= ::glGetUniformLocation(m_textureProgram, "u_colorMul");
		m_texLocSampler		= ::glGetUniformLocation(m_textureProgram, "u_tex");

		::glGenVertexArrays(1, &m_vao);
		::glBindVertexArray(m_vao);

		::glGenBuffers(1, &m_vbo);
		::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		::glGenBuffers(1, &m_ibo);
		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

		::glEnableVertexAttribArray(0);
		::glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, pos)));
		::glEnableVertexAttribArray(1);
		::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, tex)));
		::glEnableVertexAttribArray(2);
		::glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, color)));

		::glBindVertexArray(0);
	}

	Vertex2DBufferPointer CRenderer2D_GL4::createBuffer(const Vertex2D::IndexType vertexCount, const Vertex2D::IndexType indexCount)
	{
		const Vertex2D::IndexType vertexBase = static_cast<Vertex2D::IndexType>(m_vertices.size());
		const size_t indexBase = m_indices.size();

		m_vertices.resize(m_vertices.size() + vertexCount);
		m_indices.resize(indexBase + indexCount);

		return{ (m_vertices.data() + vertexBase), (m_indices.data() + indexBase), vertexBase };
	}

	GLuint CRenderer2D_GL4::glTextureOf(const Texture& texture)
	{
		return static_cast<CTexture_GL4*>(SIV3D_ENGINE(Texture))->getGLTexture(texture.id());
	}

	void CRenderer2D_GL4::addTexturedCircle(const Texture& texture, const Circle& circle, const FloatRect& uv, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedCircle(bufferCreator(), circle, uv, color, getMaxScaling()), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, color), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4])
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, colors), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedRoundRect(const Texture& texture, const FloatRect& rect, const float w, const float h, const float r, const FloatRect& uvRect, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedRoundRect(bufferCreator(), rect, w, h, r, uvRect, color, getMaxScaling()), glTextureOf(texture));
	}

	void CRenderer2D_GL4::flush()
	{
		if (m_commands.isEmpty())
		{
			m_vertices.clear();
			m_indices.clear();
			return;
		}

		::glBindVertexArray(m_vao);

		::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		::glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex2D)), m_vertices.data(), GL_STREAM_DRAW);

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		::glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indices.size() * sizeof(Vertex2D::IndexType)), m_indices.data(), GL_STREAM_DRAW);

		// vertex transform: (local * camera) then screen -> clip, packed like VSConstants2D.
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		const Mat3x2 matrix = ((m_localTransform * m_cameraTransform) * Mat3x2::Screen(SizeF{ frameBufferSize }));
		const float t0[4] = { matrix._11, matrix._12, matrix._31, matrix._32 };
		const float t1[4] = { matrix._21, matrix._22, 0.0f, 1.0f };

		::glEnable(GL_BLEND);
		::glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		size_t indexOffset = 0;

		for (const auto& command : m_commands)
		{
			if (command.texture == 0)
			{
				::glUseProgram(m_program);
				::glUniform4f(m_locTransform0, t0[0], t0[1], t0[2], t0[3]);
				::glUniform4f(m_locTransform1, t1[0], t1[1], t1[2], t1[3]);
				::glUniform4f(m_locColorMul, m_colorMul.x, m_colorMul.y, m_colorMul.z, m_colorMul.w);
			}
			else
			{
				::glUseProgram(m_textureProgram);
				::glUniform4f(m_texLocTransform0, t0[0], t0[1], t0[2], t0[3]);
				::glUniform4f(m_texLocTransform1, t1[0], t1[1], t1[2], t1[3]);
				::glUniform4f(m_texLocColorMul, m_colorMul.x, m_colorMul.y, m_colorMul.z, m_colorMul.w);
				::glActiveTexture(GL_TEXTURE0);
				::glBindTexture(GL_TEXTURE_2D, command.texture);
				::glUniform1i(m_texLocSampler, 0);
			}

			::glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.indexCount), GL_UNSIGNED_SHORT,
				reinterpret_cast<void*>(indexOffset * sizeof(Vertex2D::IndexType)));

			indexOffset += command.indexCount;
		}

		::glBindVertexArray(0);

		m_vertices.clear();
		m_indices.clear();
		m_commands.clear();
	}
}
