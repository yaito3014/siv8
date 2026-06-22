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
# include <Siv3D/EngineShader/IEngineShader.hpp>

namespace s3d
{
	// TODO(linux): load the engine GLSL shaders. Phase 0 returns empty shaders
	// (the no-op CRenderer2D_GL4 never binds them).
	class CEngineShader_GL4 final : public ISiv3DEngineShader
	{
	public:

		void init() override {}

		const VertexShader& getVS(EngineVS) const override { static const VertexShader vs; return vs; }

		const PixelShader& getPS(EnginePS) const override { static const PixelShader ps; return ps; }
	};
}
