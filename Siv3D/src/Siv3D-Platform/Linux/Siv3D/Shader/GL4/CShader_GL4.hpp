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
# include <Siv3D/Blob.hpp>
# include <Siv3D/Shader/IShader.hpp>
# include <Siv3D/AssetHandleManager/AssetHandleManager.hpp>
# include "GL4Shader.hpp"

namespace s3d
{
	// Real GLSL shader backend. Each stage is compiled as a separable program
	// (ARB_separate_shader_objects, core in GL 4.1); setVS()/setPS() bind the
	// stages independently into a program pipeline, matching the per-stage
	// ISiv3DShader interface. Constant buffers bind to UBO binding points.
	class CShader_GL4 final : public ISiv3DShader
	{
	public:

		~CShader_GL4() override;

		void init() override;

		VertexShader::IDType createVSFromReader(std::unique_ptr<IReader> reader, FilePathView pathHint, StringView entryPoint) override;
		VertexShader::IDType createVSFromSource(const std::string& source, StringView entryPoint) override;
		VertexShader::IDType createVSFromBytecode(const Blob& bytecode) override;

		PixelShader::IDType createPSFromReader(std::unique_ptr<IReader> reader, FilePathView pathHint, StringView entryPoint) override;
		PixelShader::IDType createPSFromSource(const std::string& source, StringView entryPoint) override;
		PixelShader::IDType createPSFromBytecode(const Blob& bytecode) override;

		void releaseVS(VertexShader::IDType handleID) override;
		void releasePS(PixelShader::IDType handleID) override;

		void setVS(VertexShader::IDType handleID) override;
		void setVSNull() override;
		void setPS(PixelShader::IDType handleID) override;
		void setPSNull() override;

		const Blob& getBytecodeVS(VertexShader::IDType handleID) override;
		const Blob& getBytecodePS(PixelShader::IDType handleID) override;

		void setConstantBufferVS(uint32 slot, IConstantBuffer* cb) override;
		void setConstantBufferPS(uint32 slot, IConstantBuffer* cb) override;

		// Renderer hooks (not part of ISiv3DShader): the program pipeline that
		// setVS()/setPS() configure. The renderer binds it (and glUseProgram(0))
		// when a custom shader is active.
		[[nodiscard]]
		GLuint getPipeline() const noexcept;

	private:

		GLuint m_pipeline = 0;

		AssetHandleManager<VertexShader::IDType, GL4VertexShader> m_vertexShaders{ "VertexShader" };

		AssetHandleManager<PixelShader::IDType, GL4PixelShader> m_pixelShaders{ "PixelShader" };
	};
}
