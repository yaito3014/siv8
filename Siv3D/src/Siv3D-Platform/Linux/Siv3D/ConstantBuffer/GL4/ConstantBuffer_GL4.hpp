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
# include <Siv3D/IConstantBuffer.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// A constant buffer backed by a GL uniform buffer object (UBO). The GL object
	// is created lazily on first use so that a ConstantBuffer<T> can be constructed
	// before the GL context exists; _internal_init()/_internal_update() run during
	// rendering, when the context is current.
	class ConstantBuffer_GL4 final : public IConstantBuffer
	{
	public:

		explicit ConstantBuffer_GL4(size_t size);

		~ConstantBuffer_GL4() override;

		bool _internal_init() override;

		bool _internal_update(const void* data, size_t size) override;

		[[nodiscard]]
		GLuint getHandle() const noexcept;

	private:

		GLuint m_uniformBuffer = 0;

		uint32 m_bufferSize = 0;
	};
}
