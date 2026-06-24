//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CEngineShader_GL4.hpp"
# include "GL4EngineShaders.hpp"
# include <Siv3D/VertexShader.hpp>
# include <Siv3D/PixelShader.hpp>
# include <Siv3D/EngineLog.hpp>
# include <initializer_list>

namespace s3d
{
	namespace
	{
		[[nodiscard]]
		std::string Cat(std::initializer_list<std::string_view> parts)
		{
			size_t total = 0;
			for (const auto part : parts)
			{
				total += part.size();
			}

			std::string source;
			source.reserve(total);

			for (const auto part : parts)
			{
				source.append(part);
			}

			return source;
		}
	}

	void CEngineShader_GL4::init()
	{
		LOG_SCOPED_DEBUG("CEngineShader_GL4::init()");

		using namespace detail;

		// Order MUST match the EngineVS enum.
		m_vertexShaders.clear();
		m_vertexShaders << VertexShader::GLSL(std::string{ GLSL_VS_FullScreenTriangle }, U"main");
		m_vertexShaders << VertexShader::GLSL(Cat({ GLSL_VSPrefix, GLSL_VS_Shape }), U"main");
		m_vertexShaders << VertexShader::GLSL(Cat({ GLSL_VSPrefix, GLSL_VS_QuadWarp }), U"main");

		// Order MUST match the EnginePS enum.
		m_pixelShaders.clear();
		m_pixelShaders << PixelShader::GLSL(std::string{ GLSL_PS_FullScreenTriangle }, U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_Shape }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_Texture }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_QuadWarp }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_LineDot }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_LineDash }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_LineLongDash }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_LineDashDot }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PS_LineRoundDot }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternPolkaDot }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternStripe }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternGrid }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternChecker }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternTriangle }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSPatternPrefix, GLSL_PS_PatternHexGrid }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontMSDF }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontMSDF_Outline }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontMSDF_Shadow }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontMSDF_OutlineShadow }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontMSDF_Glow }), U"main");
		m_pixelShaders << PixelShader::GLSL(Cat({ GLSL_PSPrefix, GLSL_PSMSDFPrefix, GLSL_PS_FontPrint }), U"main");

		// Surface any compile failures (createV/PSFromSource also logs the GL log).
		for (size_t i = 0; i < m_vertexShaders.size(); ++i)
		{
			if (m_vertexShaders[i].isEmpty())
			{
				LOG_FAIL(fmt::format("❌ CEngineShader_GL4: engine VS [{}] failed to compile", i));
			}
		}

		for (size_t i = 0; i < m_pixelShaders.size(); ++i)
		{
			if (m_pixelShaders[i].isEmpty())
			{
				LOG_FAIL(fmt::format("❌ CEngineShader_GL4: engine PS [{}] failed to compile", i));
			}
		}
	}

	const VertexShader& CEngineShader_GL4::getVS(const EngineVS vs) const
	{
		return m_vertexShaders[static_cast<size_t>(vs)];
	}

	const PixelShader& CEngineShader_GL4::getPS(const EnginePS ps) const
	{
		return m_pixelShaders[static_cast<size_t>(ps)];
	}
}
