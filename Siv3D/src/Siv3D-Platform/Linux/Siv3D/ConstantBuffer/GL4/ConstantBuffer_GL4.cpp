//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "ConstantBuffer_GL4.hpp"

namespace s3d
{
	ConstantBuffer_GL4::ConstantBuffer_GL4(const size_t size)
		: m_bufferSize{ static_cast<uint32>(size) } {}

	ConstantBuffer_GL4::~ConstantBuffer_GL4()
	{
		if (m_uniformBuffer)
		{
			::glDeleteBuffers(1, &m_uniformBuffer);
			m_uniformBuffer = 0;
		}
	}

	bool ConstantBuffer_GL4::_internal_init()
	{
		if (m_uniformBuffer)
		{
			return true;
		}

		::glGenBuffers(1, &m_uniformBuffer);

		if (not m_uniformBuffer)
		{
			return false;
		}

		::glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
		::glBufferData(GL_UNIFORM_BUFFER, m_bufferSize, nullptr, GL_DYNAMIC_DRAW);
		::glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool ConstantBuffer_GL4::_internal_update(const void* const data, const size_t size)
	{
		if (not _internal_init())
		{
			return false;
		}

		::glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
		::glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
		::glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	GLuint ConstantBuffer_GL4::getHandle() const noexcept
	{
		return m_uniformBuffer;
	}
}
