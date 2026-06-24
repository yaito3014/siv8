//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CShader_GL4.hpp"
# include <Siv3D/IReader.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/ConstantBuffer/GL4/ConstantBuffer_GL4.hpp>

namespace s3d
{
	namespace
	{
		[[nodiscard]]
		std::string ReadAll(IReader& reader)
		{
			const int64 size = reader.size();

			if (size <= 0)
			{
				return{};
			}

			std::string source(static_cast<size_t>(size), '\0');
			reader.read(source.data(), size);
			return source;
		}
	}

	CShader_GL4::~CShader_GL4()
	{
		LOG_SCOPED_DEBUG("CShader_GL4::~CShader_GL4()");

		m_pixelShaders.destroy();
		m_vertexShaders.destroy();

		if (m_pipeline)
		{
			::glDeleteProgramPipelines(1, &m_pipeline);
			m_pipeline = 0;
		}
	}

	void CShader_GL4::init()
	{
		LOG_SCOPED_DEBUG("CShader_GL4::init()");

		::glGenProgramPipelines(1, &m_pipeline);
		::glBindProgramPipeline(m_pipeline);

		// Null shaders (a bound stage of 0 == "no shader"), mirroring D3D11/Metal.
		m_vertexShaders.setNullData(std::make_unique<GL4VertexShader>(GL4VertexShader::Null{}));
		m_pixelShaders.setNullData(std::make_unique<GL4PixelShader>(GL4PixelShader::Null{}));
	}

	VertexShader::IDType CShader_GL4::createVSFromReader(std::unique_ptr<IReader> reader, const FilePathView, const StringView entryPoint)
	{
		return createVSFromSource(ReadAll(*reader), entryPoint);
	}

	VertexShader::IDType CShader_GL4::createVSFromSource(const std::string& source, const StringView)
	{
		auto shader = std::make_unique<GL4VertexShader>(source);

		if (not shader->isInitialized())
		{
			return VertexShader::IDType::Null();
		}

		return m_vertexShaders.add(std::move(shader));
	}

	VertexShader::IDType CShader_GL4::createVSFromBytecode(const Blob& bytecode)
	{
		// GL has no offline bytecode; the "bytecode" is GLSL source text.
		return createVSFromSource(std::string(reinterpret_cast<const char*>(bytecode.data()), bytecode.size()), {});
	}

	PixelShader::IDType CShader_GL4::createPSFromReader(std::unique_ptr<IReader> reader, const FilePathView, const StringView entryPoint)
	{
		return createPSFromSource(ReadAll(*reader), entryPoint);
	}

	PixelShader::IDType CShader_GL4::createPSFromSource(const std::string& source, const StringView)
	{
		auto shader = std::make_unique<GL4PixelShader>(source);

		if (not shader->isInitialized())
		{
			return PixelShader::IDType::Null();
		}

		return m_pixelShaders.add(std::move(shader));
	}

	PixelShader::IDType CShader_GL4::createPSFromBytecode(const Blob& bytecode)
	{
		return createPSFromSource(std::string(reinterpret_cast<const char*>(bytecode.data()), bytecode.size()), {});
	}

	void CShader_GL4::releaseVS(const VertexShader::IDType handleID)
	{
		m_vertexShaders.erase(handleID);
	}

	void CShader_GL4::releasePS(const PixelShader::IDType handleID)
	{
		m_pixelShaders.erase(handleID);
	}

	void CShader_GL4::setVS(const VertexShader::IDType handleID)
	{
		::glUseProgramStages(m_pipeline, GL_VERTEX_SHADER_BIT, m_vertexShaders[handleID]->getProgram());
	}

	void CShader_GL4::setVSNull()
	{
		::glUseProgramStages(m_pipeline, GL_VERTEX_SHADER_BIT, 0);
	}

	void CShader_GL4::setPS(const PixelShader::IDType handleID)
	{
		::glUseProgramStages(m_pipeline, GL_FRAGMENT_SHADER_BIT, m_pixelShaders[handleID]->getProgram());
	}

	void CShader_GL4::setPSNull()
	{
		::glUseProgramStages(m_pipeline, GL_FRAGMENT_SHADER_BIT, 0);
	}

	const Blob& CShader_GL4::getBytecodeVS(const VertexShader::IDType handleID)
	{
		return m_vertexShaders[handleID]->getBytecode();
	}

	const Blob& CShader_GL4::getBytecodePS(const PixelShader::IDType handleID)
	{
		return m_pixelShaders[handleID]->getBytecode();
	}

	void CShader_GL4::setConstantBufferVS(const uint32 slot, IConstantBuffer* cb)
	{
		const GLuint handle = static_cast<ConstantBuffer_GL4*>(cb)->getHandle();
		::glBindBufferBase(GL_UNIFORM_BUFFER, (VSUniformBlockBindingBase + slot), handle);
	}

	void CShader_GL4::setConstantBufferPS(const uint32 slot, IConstantBuffer* cb)
	{
		const GLuint handle = static_cast<ConstantBuffer_GL4*>(cb)->getHandle();
		::glBindBufferBase(GL_UNIFORM_BUFFER, (PSUniformBlockBindingBase + slot), handle);
	}

	GLuint CShader_GL4::getPipeline() const noexcept
	{
		return m_pipeline;
	}
}
