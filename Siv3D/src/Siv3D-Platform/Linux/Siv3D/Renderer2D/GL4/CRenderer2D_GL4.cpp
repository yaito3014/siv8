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
	vec4 t = texture(u_tex, v_uv);
	o_color = (t * v_color);
}
)";

		// MSDF text: median of the 3 distance channels, screen-space AA via fwidth.
		// (The engine premultiplies the glyph color into the vertex color.)
		constexpr StringView MSDFPSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform sampler2D u_tex;
float median(vec3 c) { return max(min(c.r, c.g), min(max(c.r, c.g), c.b)); }
void main()
{
	vec3 s = texture(u_tex, v_uv).rgb;
	float sd = median(s);
	float w = fwidth(sd);
	float a = smoothstep((0.5 - w), (0.5 + w), sd);
	o_color = (v_color * a);
}
)";

		// Patterns are computed in screen space (gl_FragCoord) transformed by the
		// packed uvTransform; mask selects primary (vertex color) vs background.
		// Matches Pattern::PolkaDot(0)/Grid(2)/Checker(3); others TODO(linux).
		constexpr StringView PatternPSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform vec4 u_pt0;
uniform vec4 u_pt1;
uniform vec4 u_patBg;
uniform int u_patType;
void main()
{
	vec2 f = gl_FragCoord.xy;
	vec2 c = vec2(u_pt0.z, u_pt0.w) + (f.x * vec2(u_pt0.x, u_pt0.y)) + (f.y * vec2(u_pt1.x, u_pt1.y));
	float p0 = u_pt1.z;
	float mask = 0.0;
	if (u_patType == 3) { vec2 cc = floor(c); mask = mod((cc.x + cc.y), 2.0); }				// Checker
	else if (u_patType == 2) { vec2 d = min(fract(c), (vec2(1.0) - fract(c))); mask = (((d.x < p0) || (d.y < p0)) ? 1.0 : 0.0); } // Grid
	else if (u_patType == 0) { float dd = length(fract(c) - vec2(0.5)); mask = ((dd < p0) ? 1.0 : 0.0); } // PolkaDot
	else if (u_patType == 1) { mask = ((fract(c.x) < (p0 * 2.0)) ? 1.0 : 0.0); }				// Stripe
	else if (u_patType == 4) { float ta = (c.x - c.y / 1.7320508); float tb = (c.y * 2.0 / 1.7320508); mask = (((fract(ta) + fract(tb)) < 1.0) ? 1.0 : 0.0); } // Triangle (equilateral tiling)
	else if (u_patType == 5) {																	// HexGrid (border lines)
		vec2 rr = vec2(1.0, 1.7320508);
		vec2 hh = (rr * 0.5);
		vec2 aa = (mod(c, rr) - hh);
		vec2 bb = (mod((c - hh), rr) - hh);
		vec2 gv = ((dot(aa, aa) < dot(bb, bb)) ? aa : bb);
		vec2 q = abs(gv);
		float hd = max(dot(q, normalize(vec2(1.0, 1.7320508))), q.x);
		mask = ((hd > p0) ? 1.0 : 0.0);
	}
	vec4 bg = vec4((u_patBg.rgb * u_patBg.a), u_patBg.a);
	o_color = mix(bg, v_color, mask);
}
)";

		// Line styles: v_uv.x is distance along the line in thickness-units (phase
		// already folded in via dotOffset), v_uv.y is across [0,1]. Hard-edged
		// dash/dot patterns; matches LineType Dotted(1)/Dashed(2)/LongDash(3)/
		// DashDot(4)/RoundDot(5).
		constexpr StringView LinePSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform int u_lineType;
void main()
{
	float u = v_uv.x;
	float a = 1.0;
	if (u_lineType == 1) { a = ((mod(u, 2.0) < 1.0) ? 1.0 : 0.0); }
	else if (u_lineType == 2) { a = ((mod(u, 4.0) < 2.0) ? 1.0 : 0.0); }
	else if (u_lineType == 3) { a = ((mod(u, 6.0) < 4.0) ? 1.0 : 0.0); }
	else if (u_lineType == 4) { float m = mod(u, 6.0); a = (((m < 3.0) || ((m > 4.0) && (m < 5.0))) ? 1.0 : 0.0); }
	else if (u_lineType == 5) { float t = abs(1.0 - mod(u, 2.0)); float across = (abs(v_uv.y - 0.5) * 2.0); a = (((t * t + across * across) < 1.0) ? 1.0 : 0.0); }
	o_color = (v_color * a);
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

		if (m_lineProgram) { ::glDeleteProgram(m_lineProgram); }
		if (m_patternProgram) { ::glDeleteProgram(m_patternProgram); }
		if (m_msdfProgram) { ::glDeleteProgram(m_msdfProgram); }
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

		m_msdfProgram = LinkProgram(MSDFPSCode);
		m_msdfLocTransform0	= ::glGetUniformLocation(m_msdfProgram, "u_t0");
		m_msdfLocTransform1	= ::glGetUniformLocation(m_msdfProgram, "u_t1");
		m_msdfLocColorMul	= ::glGetUniformLocation(m_msdfProgram, "u_colorMul");
		m_msdfLocSampler	= ::glGetUniformLocation(m_msdfProgram, "u_tex");

		m_patternProgram = LinkProgram(PatternPSCode);
		m_patLocTransform0	= ::glGetUniformLocation(m_patternProgram, "u_t0");
		m_patLocTransform1	= ::glGetUniformLocation(m_patternProgram, "u_t1");
		m_patLocColorMul	= ::glGetUniformLocation(m_patternProgram, "u_colorMul");
		m_patLocPt0			= ::glGetUniformLocation(m_patternProgram, "u_pt0");
		m_patLocPt1			= ::glGetUniformLocation(m_patternProgram, "u_pt1");
		m_patLocBg			= ::glGetUniformLocation(m_patternProgram, "u_patBg");
		m_patLocType		= ::glGetUniformLocation(m_patternProgram, "u_patType");

		m_lineProgram = LinkProgram(LinePSCode);
		m_lineLocTransform0	= ::glGetUniformLocation(m_lineProgram, "u_t0");
		m_lineLocTransform1	= ::glGetUniformLocation(m_lineProgram, "u_t1");
		m_lineLocColorMul	= ::glGetUniformLocation(m_lineProgram, "u_colorMul");
		m_lineLocType		= ::glGetUniformLocation(m_lineProgram, "u_lineType");

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
		pushCommand(Vertex2DBuilder::BuildTexturedCircle(bufferCreator(), circle, uv, color, getMaxScaling()), (m_customPSActive ? Program::MSDF : Program::Texture), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, color), (m_customPSActive ? Program::MSDF : Program::Texture), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4])
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, colors), (m_customPSActive ? Program::MSDF : Program::Texture), glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedRoundRect(const Texture& texture, const FloatRect& rect, const float w, const float h, const float r, const FloatRect& uvRect, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedRoundRect(bufferCreator(), rect, w, h, r, uvRect, color, getMaxScaling()), (m_customPSActive ? Program::MSDF : Program::Texture), glTextureOf(texture));
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
			GLuint program = 0;
			GLint locT0 = -1, locT1 = -1, locColorMul = -1, locSampler = -1;

			switch (command.program)
			{
			case Program::Shape:
				program = m_program; locT0 = m_locTransform0; locT1 = m_locTransform1; locColorMul = m_locColorMul;
				break;
			case Program::Texture:
				program = m_textureProgram; locT0 = m_texLocTransform0; locT1 = m_texLocTransform1; locColorMul = m_texLocColorMul; locSampler = m_texLocSampler;
				break;
			case Program::MSDF:
				program = m_msdfProgram; locT0 = m_msdfLocTransform0; locT1 = m_msdfLocTransform1; locColorMul = m_msdfLocColorMul; locSampler = m_msdfLocSampler;
				break;
			case Program::Pattern:
				program = m_patternProgram; locT0 = m_patLocTransform0; locT1 = m_patLocTransform1; locColorMul = m_patLocColorMul;
				break;
			case Program::Line:
				program = m_lineProgram; locT0 = m_lineLocTransform0; locT1 = m_lineLocTransform1; locColorMul = m_lineLocColorMul;
				break;
			}

			::glUseProgram(program);
			::glUniform4f(locT0, t0[0], t0[1], t0[2], t0[3]);
			::glUniform4f(locT1, t1[0], t1[1], t1[2], t1[3]);
			::glUniform4f(locColorMul, m_colorMul.x, m_colorMul.y, m_colorMul.z, m_colorMul.w);

			if (command.program == Program::Pattern)
			{
				const auto& p = command.patternParams;
				::glUniform4f(m_patLocPt0, p[0].x, p[0].y, p[0].z, p[0].w);
				::glUniform4f(m_patLocPt1, p[1].x, p[1].y, p[1].z, p[1].w);
				::glUniform4f(m_patLocBg, p[2].x, p[2].y, p[2].z, p[2].w);
				::glUniform1i(m_patLocType, command.patternType);
			}
			else if (command.program == Program::Line)
			{
				::glUniform1i(m_lineLocType, command.patternType);
			}

			if (command.texture != 0)
			{
				::glActiveTexture(GL_TEXTURE0);
				::glBindTexture(GL_TEXTURE_2D, command.texture);
				::glUniform1i(locSampler, 0);
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
