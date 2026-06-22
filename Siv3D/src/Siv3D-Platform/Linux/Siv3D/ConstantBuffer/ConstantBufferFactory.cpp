//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/IConstantBuffer.hpp>

namespace s3d
{
	namespace
	{
		// TODO(linux): real GL uniform-buffer object. Phase 0 holds nothing.
		class ConstantBuffer_GL4 final : public IConstantBuffer
		{
		public:

			explicit ConstantBuffer_GL4(size_t) {}

			bool _internal_init() override { return true; }

			bool _internal_update(const void*, size_t) override { return true; }
		};
	}

	std::unique_ptr<IConstantBuffer> IConstantBuffer::Create(const size_t size)
	{
		return std::make_unique<ConstantBuffer_GL4>(size);
	}
}
