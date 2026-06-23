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
	// Returns empty shaders. CRenderer2D_GL4 compiles and binds its own inline
	// GLSL programs directly, so this engine-shader registry is bypassed rather
	// than used; nothing depends on it yet (TODO(linux): wire it up if a future
	// subsystem needs the shared engine shaders through this interface).
	class CEngineShader_GL4 final : public ISiv3DEngineShader
	{
	public:

		void init() override {}

		const VertexShader& getVS(EngineVS) const override { static const VertexShader vs; return vs; }

		const PixelShader& getPS(EnginePS) const override { static const PixelShader ps; return ps; }
	};
}
