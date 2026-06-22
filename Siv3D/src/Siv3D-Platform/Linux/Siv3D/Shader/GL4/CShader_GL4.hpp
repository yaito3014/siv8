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
# include <Siv3D/Shader/IShader.hpp>
# include <Siv3D/Blob.hpp>

namespace s3d
{
	// TODO(linux): real GLSL shader compilation/binding. Phase 0 no-op.
	class CShader_GL4 final : public ISiv3DShader
	{
	public:

		void init() override {}
		VertexShader::IDType createVSFromReader(std::unique_ptr<IReader> reader, FilePathView pathHint, StringView entryPoint) override { return {}; }
		VertexShader::IDType createVSFromSource(const std::string& source, StringView entryPoint) override { return {}; }
		VertexShader::IDType createVSFromBytecode(const Blob& bytecode) override { return {}; }
		PixelShader::IDType createPSFromReader(std::unique_ptr<IReader> reader, FilePathView pathHint, StringView entryPoint) override { return {}; }
		PixelShader::IDType createPSFromSource(const std::string& source, StringView entryPoint) override { return {}; }
		PixelShader::IDType createPSFromBytecode(const Blob& bytecode) override { return {}; }
		void releaseVS(VertexShader::IDType handleID) override {}
		void releasePS(PixelShader::IDType handleID) override {}
		void setVS(VertexShader::IDType handleID) override {}
		void setVSNull() override {}
		void setPS(PixelShader::IDType handleID) override {}
		void setPSNull() override {}
		const Blob& getBytecodeVS(VertexShader::IDType handleID) override { static const Blob b; return b; }
		const Blob& getBytecodePS(PixelShader::IDType handleID) override { static const Blob b; return b; }
		void setConstantBufferVS(uint32 slot, IConstantBuffer* cb) override {}
		void setConstantBufferPS(uint32 slot, IConstantBuffer* cb) override {}
	};
}
