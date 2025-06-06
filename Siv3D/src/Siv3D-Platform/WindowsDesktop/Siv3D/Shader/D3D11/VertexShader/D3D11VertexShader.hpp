﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2025 Ryo Suzuki
//	Copyright (c) 2016-2025 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/Common.hpp>
# include <Siv3D/Blob.hpp>
# include <Siv3D/Renderer/D3D11/D3D11.hpp>

namespace s3d
{
	class D3D11VertexShader
	{
	public:

		struct Null {};

		[[nodiscard]]
		D3D11VertexShader() = default;

		[[nodiscard]]
		D3D11VertexShader(Null);

		[[nodiscard]]
		D3D11VertexShader(Blob&& bytecode, ID3D11Device* device);

		[[nodiscard]]
		bool isInitialized() const noexcept;

		[[nodiscard]]
		ID3D11VertexShader* getShader() const;

		[[nodiscard]]
		const Blob& getBytecode() const noexcept;

	private:

		Blob m_bytecode;

		ComPtr<ID3D11VertexShader> m_shader;

		bool m_initialized = false;
	};
}
