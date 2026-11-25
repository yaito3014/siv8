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
# include <Siv3D/Mat3x3.hpp>

namespace s3d
{
	namespace
	{
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
		static EnginePS PatternPS(const PatternType pattern)
		{
			switch (pattern)
			{
			case PatternType::PolkaDot:	return EnginePS::PatternPolkaDot;
			case PatternType::Stripe:	return EnginePS::PatternStripe;
			case PatternType::Grid:		return EnginePS::PatternGrid;
			case PatternType::Checker:	return EnginePS::PatternChecker;
			case PatternType::Triangle:	return EnginePS::PatternTriangle;
			case PatternType::HexGrid:	return EnginePS::PatternHexGrid;
			case PatternType::Halftone:	return EnginePS::PatternHalftone;
			case PatternType::Wave:		return EnginePS::PatternWave;
			case PatternType::Ripple:	return EnginePS::PatternRipple;
			case PatternType::Weave:	return EnginePS::PatternWeave;
			case PatternType::Truchet:	return EnginePS::PatternTruchet;
			default:					return EnginePS::Shape2D;
			}
		}

		[[nodiscard]]
		static GLint ToGLMagFilter(const TextureFilter f)
		{
			return ((f == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR);
		}

		// Textures cap GL_TEXTURE_MAX_LEVEL at their real mip count, so a mipmapped
		// min filter is valid for single-level textures too.
		[[nodiscard]]
		static GLint ToGLMinFilter(const TextureFilter minFilter, const TextureFilter mipFilter)
		{
			if (minFilter == TextureFilter::Nearest)
			{
				return ((mipFilter == TextureFilter::Nearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR);
			}
			else
			{
				return ((mipFilter == TextureFilter::Nearest) ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
			}
		}

		[[nodiscard]]
		static std::array<GLfloat, 4> ToGLBorderColor(const SamplerBorderColor borderColor) noexcept
		{
			switch (borderColor)
			{
			case SamplerBorderColor::OpaqueBlack:	return{ 0.0f, 0.0f, 0.0f, 1.0f };
			case SamplerBorderColor::OpaqueWhite:	return{ 1.0f, 1.0f, 1.0f, 1.0f };
			default:								return{ 0.0f, 0.0f, 0.0f, 0.0f };
			}
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
		::glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, ToGLMinFilter(state.minFilter, state.mipFilter));
		::glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, ToGLMagFilter(state.magFilter));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, ToGLWrap(state.uAddressMode));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, ToGLWrap(state.vAddressMode));
		::glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, ToGLWrap(state.wAddressMode));
		::glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, state.minLOD);

		const std::array<GLfloat, 4> borderColor = ToGLBorderColor(state.borderColor);
		::glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, borderColor.data());

		// Like D3D11, anisotropy applies only to an all-linear filter.
		if ((1 < state.maxAnisotropy) && (0.0f < m_maxAnisotropy)
			&& (state.minFilter == TextureFilter::Linear) && (state.magFilter == TextureFilter::Linear) && (state.mipFilter == TextureFilter::Linear))
		{
			::glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, Min(static_cast<float>(state.maxAnisotropy), m_maxAnisotropy));
		}
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

		if (HasGLExtension("GL_ARB_texture_filter_anisotropic") || HasGLExtension("GL_EXT_texture_filter_anisotropic"))
		{
			::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_maxAnisotropy);
		}

		// Box-shadow sprite for addCircleShadow/addRectShadow/addRoundRectShadow.
		{
			const Image boxShadowImage{ Resource(U"engine/texture/box-shadow/256.png") };

			const Array<Image> boxShadowImageMips =
			{
				Image{ Resource(U"engine/texture/box-shadow/128.png") },
				Image{ Resource(U"engine/texture/box-shadow/64.png") },
				Image{ Resource(U"engine/texture/box-shadow/32.png") },
				Image{ Resource(U"engine/texture/box-shadow/16.png") },
				Image{ Resource(U"engine/texture/box-shadow/8.png") },
			};

			m_shadowTexture = std::make_unique<Texture>(boxShadowImage, boxShadowImageMips);
		}

		if (m_shadowTexture->isEmpty())
		{
			throw InternalEngineError{ "Failed to create a box-shadow texture" };
		}
	}

	Vertex2DBufferPointer CRenderer2D_GL4::createBuffer(const uint16 vertexCount, const uint32 indexCount)
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
		pushCommand(Vertex2DBuilder::BuildTexturedCircle(bufferCreator(), circle, uv, color, getRMSScaling()), Program::Texture, glTextureOf(texture));
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
		pushCommand(Vertex2DBuilder::BuildTexturedRoundRect(bufferCreator(), rect, w, h, r, uvRect, color, getRMSScaling()), Program::Texture, glTextureOf(texture));
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

		// Each command carries its (local * camera) * quad-warp transform; the
		// screen->clip part is folded in per command since a custom viewport changes
		// the screen mapping.
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;

		// Bind the program pipeline + engine constant buffers (UBOs) once for the
		// batch; per command we update the UBO data and swap the pipeline's stages.
		auto* const shader = static_cast<CShader_GL4*>(SIV3D_ENGINE(Shader));
		auto* const engineShader = SIV3D_ENGINE(EngineShader);
		const VertexShader::IDType shapeVS = engineShader->getVS(EngineVS::Shape2D).id();
		const VertexShader::IDType patternVS = engineShader->getVS(EngineVS::Pattern2D).id();

		const auto enginePSFor = [&](const DrawCommand& cmd) -> PixelShader::IDType
		{
			EnginePS ps = EnginePS::Shape2D;
			switch (cmd.program)
			{
			case Program::Shape:	ps = EnginePS::Shape2D; break;
			case Program::Texture:	ps = EnginePS::Texture2D; break;
			case Program::MSDF:		ps = EnginePS::FontMSDF; break;
			case Program::Pattern:	ps = PatternPS(static_cast<PatternType>(cmd.patternType)); break;
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

			// --- constants (UBOs) ---
			m_vsConstants->setTransform(command.transform * Mat3x3{ Mat3x2::Screen(vp.size.x, vp.size.y) });
			m_vsConstants->colorMul = m_colorMul;

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
			const VertexShader::IDType vsID = command.customVS ? *command.customVS
				: ((command.program == Program::Pattern) ? patternVS : shapeVS);
			shader->setVS(vsID);
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
