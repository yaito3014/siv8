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
# include <Siv3D/Array.hpp>
# include <Siv3D/EngineShader/IEngineShader.hpp>

namespace s3d
{
	// Compiles the engine's built-in 2D GLSL shaders (GL4EngineShaders.hpp) and
	// serves them through the registry, indexed by the EngineVS / EnginePS enums
	// (mirrors CEngineShader_D3D11 / CEngineShader_Metal). CRenderer2D_GL4 selects
	// shaders from here and binds them via the shader engine + program pipeline.
	class CEngineShader_GL4 final : public ISiv3DEngineShader
	{
	public:

		void init() override;

		const VertexShader& getVS(EngineVS vs) const override;

		const PixelShader& getPS(EnginePS ps) const override;

	private:

		Array<VertexShader> m_vertexShaders;

		Array<PixelShader> m_pixelShaders;
	};
}
