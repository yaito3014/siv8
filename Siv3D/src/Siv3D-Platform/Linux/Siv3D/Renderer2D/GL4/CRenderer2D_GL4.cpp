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
# include <Siv3D/Image.hpp>
# include <Siv3D/Resource.hpp>
# include <Siv3D/Unicode.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/Texture/ITexture.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/Error/InternalEngineError.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Texture/GL4/CTexture_GL4.hpp>
# include <Siv3D/Shader/IShader.hpp>
# include <Siv3D/Shader/GL4/CShader_GL4.hpp>
# include <Siv3D/EngineShader/IEngineShader.hpp>

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
		// packed uvTransform, then mixed between primary (vertex color) and
		// background. These are faithful ports of the engine's reference pattern
		// shaders (engine/shader/{metal,d3d11}/2d.* — PS_PatternPolkaDot etc.), so
		// shape, sizing and smoothstep edge-AA match D3D11/Metal exactly. u_pt0/u_pt1
		// are the packed uvTransform (u_pt1.zw = param0/param1); u_patType is the
		// PatternType enum (PolkaDot=0, Stripe=1, Grid=2, Checker=3, Triangle=4,
		// HexGrid=5). gl_FragCoord.y is flipped to the reference's top-left origin.
		constexpr StringView PatternPSCode =
UR"(#version 410 core
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
uniform vec4 u_pt0;          // (m11, m12, m31, m32)
uniform vec4 u_pt1;          // (m21, m22, param0, param1)
uniform vec4 u_patBg;        // background color (straight alpha)
uniform int u_patType;
uniform float u_patFbHeight; // framebuffer height, for gl_FragCoord.y flip

vec2 patUV(vec2 f)
{
	return vec2(u_pt0.z, u_pt0.w) + (f.x * vec2(u_pt0.x, u_pt0.y)) + (f.y * vec2(u_pt1.x, u_pt1.y));
}
vec2 patIntegral(vec2 v)
{
	v /= 2.0;
	return (floor(v) + max((2.0 * fract(v) - 1.0), 0.0));
}
float patChecker(vec2 p, vec2 hv)
{
	vec2 fw = fwidth(p);
	float w = max(fw.x, fw.y);
	vec2 i = (patIntegral(p + (0.5 * w)) - patIntegral(p - (0.5 * w)));
	i *= hv;
	i /= w;
	return (i.x + i.y - (2.0 * i.x * i.y));
}
vec2 patSkew(vec2 v) // square lattice -> equilateral-triangle lattice
{
	return vec2((v.x + (v.y * 0.57735027)), (v.y * 1.15470054));
}
float patHex(vec2 p)
{
	vec2 HEX = vec2(1.0, 1.73205081);
	vec4 t = (floor(vec4(p, (p - vec2(0.5, 1.0))) / HEX.xyxy) + vec4(0.5));
	vec4 h = vec4((p - (t.xy * HEX)), (p - ((t.zw + vec2(0.5)) * HEX)));
	vec2 hx = abs((dot(h.xy, h.xy) < dot(h.zw, h.zw)) ? h.xy : h.zw);
	return max(dot(hx, (HEX * 0.5)), hx.x);
}
void main()
{
	vec2 f = vec2(gl_FragCoord.x, (u_patFbHeight - gl_FragCoord.y));
	vec2 uv = patUV(f);
	float p0 = u_pt1.z;
	float p1 = u_pt1.w;
	vec4 primary = v_color;                                 // premultiplied in the VS
	vec4 bg = vec4((u_patBg.rgb * u_patBg.a), u_patBg.a);    // premultiply background

	if (u_patType == 0)            // PolkaDot
	{
		float value = length((2.0 * fract(uv)) - 1.0);
		float fw = (length(vec2(dFdx(value), dFdy(value))) * 0.70710678);
		float c = smoothstep((p0 - fw), (p0 + fw), value);
		o_color = mix(primary, bg, c);
	}
	else if (u_patType == 1)       // Stripe
	{
		float u = uv.x;
		float fw = fwidth(u);
		float value = abs((2.0 * fract(u)) - 1.0);
		float ts = ((p0 * (1.0 + (2.0 * fw))) - fw);
		o_color = mix(primary, bg, smoothstep((ts - fw), (ts + fw), value));
	}
	else if (u_patType == 2)       // Grid
	{
		vec2 fw = fwidth(uv);
		vec2 value = abs((2.0 * fract(uv)) - 1.0);
		vec2 ts = ((vec2(p0) * (vec2(1.0) + fw)) - fw);
		vec2 t1 = smoothstep((ts - fw), (ts + fw), value);
		o_color = mix(primary, bg, min(t1.x, t1.y));
	}
	else if (u_patType == 3)       // Checker
	{
		o_color = mix(primary, bg, patChecker(uv, vec2(p0, p1)));
	}
	else if (u_patType == 4)       // Triangle
	{
		vec2 fw = (fwidth(uv) * 0.25);
		vec2 s1 = patSkew(uv + vec2(-fw.x, -fw.y));
		vec2 s2 = patSkew(uv + vec2( fw.x,  fw.y));
		vec2 s3 = patSkew(uv + vec2(-fw.x,  fw.y));
		vec2 s4 = patSkew(uv + vec2( fw.x, -fw.y));
		vec4 fa = fract(vec4(s1, s2));
		vec4 fb = fract(vec4(s3, s4));
		vec4 ss = vec4(step(fa.x, fa.y), step(fa.z, fa.w), step(fb.x, fb.y), step(fb.z, fb.w));
		o_color = mix(primary, bg, dot(ss, vec4(0.25)));
	}
	else                           // HexGrid (5)
	{
		vec2 fw = fwidth(uv);
		float w = (max(fw.x, fw.y) * 0.5);
		float ts = (p0 * (1.0 + (2.0 * w)));
		o_color = mix(bg, primary, smoothstep((ts - w), (ts + w), patHex(uv)));
	}
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
		static GLenum ToGLBlendFactor(const BlendFactor f)
		{
			switch (f)
			{
			case BlendFactor::Zero:						return GL_ZERO;
			case BlendFactor::One:						return GL_ONE;
			case BlendFactor::SourceColor:				return GL_SRC_COLOR;
			case BlendFactor::OneMinusSourceColor:		return GL_ONE_MINUS_SRC_COLOR;
			case BlendFactor::SourceAlpha:				return GL_SRC_ALPHA;
			case BlendFactor::OneMinusSourceAlpha:		return GL_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::DestinationAlpha:			return GL_DST_ALPHA;
			case BlendFactor::OneMinusDestinationAlpha:	return GL_ONE_MINUS_DST_ALPHA;
			case BlendFactor::DestinationColor:			return GL_DST_COLOR;
			case BlendFactor::OneMinusDestinationColor:	return GL_ONE_MINUS_DST_COLOR;
			case BlendFactor::SourceAlphaSaturated:		return GL_SRC_ALPHA_SATURATE;
			case BlendFactor::BlendColor:				return GL_CONSTANT_COLOR;
			case BlendFactor::OneMinusBlendColor:		return GL_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactor::Source1Color:				return GL_SRC1_COLOR;
			case BlendFactor::OneMinusSource1Color:		return GL_ONE_MINUS_SRC1_COLOR;
			case BlendFactor::Source1Alpha:				return GL_SRC1_ALPHA;
			case BlendFactor::OneMinusSource1Alpha:		return GL_ONE_MINUS_SRC1_ALPHA;
			default:									return GL_ONE;
			}
		}

		[[nodiscard]]
		static GLenum ToGLBlendOp(const BlendOperation op)
		{
			switch (op)
			{
			case BlendOperation::Add:				return GL_FUNC_ADD;
			case BlendOperation::Subtract:			return GL_FUNC_SUBTRACT;
			case BlendOperation::ReverseSubtract:	return GL_FUNC_REVERSE_SUBTRACT;
			case BlendOperation::Min:				return GL_MIN;
			case BlendOperation::Max:				return GL_MAX;
			default:								return GL_FUNC_ADD;
			}
		}

		[[nodiscard]]
		static GLint ToGLWrap(const TextureAddressMode m)
		{
			switch (m)
			{
			case TextureAddressMode::Repeat:		return GL_REPEAT;
			case TextureAddressMode::Mirror:		return GL_MIRRORED_REPEAT;
			case TextureAddressMode::Clamp:			return GL_CLAMP_TO_EDGE;
			case TextureAddressMode::BorderColor:	return GL_CLAMP_TO_BORDER;
			case TextureAddressMode::MirrorClamp:	return GL_CLAMP_TO_EDGE; // GL_MIRROR_CLAMP_TO_EDGE is 4.4+
			default:								return GL_CLAMP_TO_EDGE;
			}
		}

		[[nodiscard]]
		static GLint ToGLFilter(const TextureFilter f)
		{
			// Textures carry no mip chain on this backend, so use the non-mipmapped
			// variants for both min and mag (a mipmap min filter would sample black).
			return ((f == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR);
		}

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

		for (auto& [key, sampler] : m_samplerCache) { ::glDeleteSamplers(1, &sampler); }
		if (m_ibo) { ::glDeleteBuffers(1, &m_ibo); }
		if (m_vbo) { ::glDeleteBuffers(1, &m_vbo); }
		if (m_vao) { ::glDeleteVertexArrays(1, &m_vao); }
	}

	GLuint CRenderer2D_GL4::samplerObjectFor(const SamplerState& state)
	{
		const uint64 key = state.asValue();

		if (const auto it = m_samplerCache.find(key); it != m_samplerCache.end())
		{
			return it->second;
		}

		GLuint sampler = 0;
		::glGenSamplers(1, &sampler);
		::glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, ToGLFilter(state.minFilter));
		::glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, ToGLFilter(state.magFilter));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, ToGLWrap(state.uAddressMode));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, ToGLWrap(state.vAddressMode));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, ToGLWrap(state.wAddressMode));
		m_samplerCache.emplace(key, sampler);
		return sampler;
	}

	void CRenderer2D_GL4::init()
	{
		LOG_SCOPED_DEBUG("CRenderer2D_GL4::init()");

		// Shaders come from the engine-shader registry (CEngineShader_GL4), compiled
		// in CRenderer_GL4::init(); flush() binds them via the program pipeline + UBOs.

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

		// Box-shadow sprite for addCircleShadow/addRectShadow/addRoundRectShadow.
		// (The Linux texture backend uploads the base image only; the mip pyramid
		// the D3D11/Metal path supplies is ignored, so just load the 256px base.)
		m_shadowTexture = std::make_unique<Texture>(Image{ Resource(U"engine/texture/box-shadow/256.png") });

		if (m_shadowTexture->isEmpty())
		{
			throw InternalEngineError{ "Failed to create a box-shadow texture" };
		}
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
		pushCommand(Vertex2DBuilder::BuildTexturedCircle(bufferCreator(), circle, uv, color, getMaxScaling()), Program::Texture, glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, color), Program::Texture, glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4])
	{
		pushCommand(Vertex2DBuilder::BuildTexturedQuad(bufferCreator(), quad, uv, colors), Program::Texture, glTextureOf(texture));
	}

	void CRenderer2D_GL4::addTexturedRoundRect(const Texture& texture, const FloatRect& rect, const float w, const float h, const float r, const FloatRect& uvRect, const Float4& color)
	{
		pushCommand(Vertex2DBuilder::BuildTexturedRoundRect(bufferCreator(), rect, w, h, r, uvRect, color, getMaxScaling()), Program::Texture, glTextureOf(texture));
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

		// vertex transform base: (local * camera); the screen->clip part is folded in
		// per-command since a custom viewport changes the screen mapping.
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		const Mat3x2 baseMatrix = (m_localTransform * m_cameraTransform);

		// Bind the program pipeline + engine constant buffers (UBOs) once for the
		// batch; per command we update the UBO data and swap the pipeline's stages.
		auto* const shader = static_cast<CShader_GL4*>(SIV3D_ENGINE(Shader));
		auto* const engineShader = SIV3D_ENGINE(EngineShader);
		const VertexShader::IDType shapeVS = engineShader->getVS(EngineVS::Shape2D).id();

		const auto enginePSFor = [&](const DrawCommand& cmd) -> PixelShader::IDType
		{
			EnginePS ps = EnginePS::Shape2D;
			switch (cmd.program)
			{
			case Program::Shape:	ps = EnginePS::Shape2D; break;
			case Program::Texture:	ps = EnginePS::Texture2D; break;
			case Program::MSDF:		ps = EnginePS::FontMSDF; break;
			case Program::Pattern:	ps = static_cast<EnginePS>(static_cast<size_t>(EnginePS::PatternPolkaDot) + cmd.patternType); break;
			case Program::Line:		ps = static_cast<EnginePS>(static_cast<size_t>(EnginePS::LineDot) + (cmd.patternType - 1)); break;
			}
			return engineShader->getPS(ps).id();
		};

		::glUseProgram(0);
		::glBindProgramPipeline(shader->getPipeline());
		shader->setConstantBufferVS(0, m_vsConstants._base());
		shader->setConstantBufferPS(0, m_psConstants._base());
		shader->setConstantBufferPS(1, m_psEffectConstants._base());

		size_t indexOffset = 0;

		for (const auto& command : m_commands)
		{
			// --- render state ---
			const BlendState& bs = command.blend;
			if (bs.enabled) { ::glEnable(GL_BLEND); } else { ::glDisable(GL_BLEND); }
			::glBlendFuncSeparate(ToGLBlendFactor(bs.sourceRGB), ToGLBlendFactor(bs.destinationRGB),
				ToGLBlendFactor(bs.sourceAlpha), ToGLBlendFactor(bs.destinationAlpha));
			::glBlendEquationSeparate(ToGLBlendOp(bs.rgbOperation), ToGLBlendOp(bs.alphaOperation));
			::glColorMask(bs.writeR, bs.writeG, bs.writeB, bs.writeA);
			if (bs.alphaToCoverageEnabled) { ::glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE); } else { ::glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE); }

			const RasterizerState& rs = command.rasterizer;
			switch (rs.cullMode)
			{
			case CullMode::None:	::glDisable(GL_CULL_FACE); break;
			case CullMode::Front:	::glEnable(GL_CULL_FACE); ::glCullFace(GL_FRONT); break;
			case CullMode::Back:	::glEnable(GL_CULL_FACE); ::glCullFace(GL_BACK); break;
			}
			::glPolygonMode(GL_FRONT_AND_BACK, ((rs.triangleFillMode == TriangleFillMode::Wireframe) ? GL_LINE : GL_FILL));

			// viewport (GL window-space origin is bottom-left, so flip Y); the screen
			// matrix maps scene (0,0)-(w,h) into it, like the D3D11 backend.
			const Rect vp = command.viewport.value_or(Rect{ 0, 0, frameBufferSize.x, frameBufferSize.y });
			::glViewport(vp.pos.x, (frameBufferSize.y - (vp.pos.y + vp.size.y)), vp.size.x, vp.size.y);

			if (command.scissor)
			{
				const Rect& sc = *command.scissor;
				::glEnable(GL_SCISSOR_TEST);
				::glScissor(sc.pos.x, (frameBufferSize.y - (sc.pos.y + sc.size.y)), sc.size.x, sc.size.y);
			}
			else
			{
				::glDisable(GL_SCISSOR_TEST);
			}

			const Mat3x2 matrix = (baseMatrix * Mat3x2::Screen(vp.size.x, vp.size.y));

			// --- constants (UBOs) ---
			m_vsConstants->transform[0]	= Float4{ matrix._11, matrix._12, matrix._31, matrix._32 };
			m_vsConstants->transform[1]	= Float4{ matrix._21, matrix._22, 0.0f, 1.0f };
			m_vsConstants->colorMul		= m_colorMul;

			m_psConstants->colorAdd				= Float4{ m_colorAdd.x, m_colorAdd.y, m_colorAdd.z, 0.0f };
			m_psConstants->sdfParam				= m_sdfParams[0];
			m_psConstants->sdfOuterColorPMA		= m_sdfParams[1];
			m_psConstants->sdfShadowColorPMA	= m_sdfParams[2];

			if (command.program == Program::Pattern)
			{
				m_psEffectConstants->setPattern(command.patternParams);
			}

			m_vsConstants._update_if_dirty();
			m_psConstants._update_if_dirty();
			m_psEffectConstants._update_if_dirty();

			// --- shaders: engine registry, or the command's captured custom shader ---
			shader->setVS(command.customVS.value_or(shapeVS));
			shader->setPS(command.customPS.value_or(enginePSFor(command)));

			// --- texture / sampler (unit 0; the GLSL sampler defaults to unit 0) ---
			if (command.texture != 0)
			{
				::glActiveTexture(GL_TEXTURE0);
				::glBindTexture(GL_TEXTURE_2D, command.texture);
				::glBindSampler(0, samplerObjectFor(command.sampler));
			}
			else
			{
				::glBindSampler(0, 0);
			}

			::glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.indexCount), GL_UNSIGNED_SHORT,
				reinterpret_cast<void*>(indexOffset * sizeof(Vertex2D::IndexType)));

			indexOffset += command.indexCount;
		}

		// Restore default GL state so the next frame's clear/draw isn't affected
		// (glClear honors scissor + color mask).
		::glBindSampler(0, 0);
		::glBindProgramPipeline(0);
		::glDisable(GL_SCISSOR_TEST);
		::glDisable(GL_CULL_FACE);
		::glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		::glViewport(0, 0, frameBufferSize.x, frameBufferSize.y);

		::glBindVertexArray(0);

		m_vertices.clear();
		m_indices.clear();
		m_commands.clear();
	}
}
