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
# include <Siv3D/Texture.hpp>
# include <Siv3D/Texture/ITexture.hpp>
# include <Siv3D/Renderer/D3D11/D3D11.hpp>
# include <Siv3D/AssetHandleManager/AssetHandleManager.hpp>
# include "D3D11Texture.hpp"

namespace s3d
{
	class CRenderer_D3D11;

	class CTexture_D3D11 final : public ISiv3DTexture
	{
	public:

		static constexpr size_t RenderTextureFormatCount = 14;

		CTexture_D3D11();

		~CTexture_D3D11() override;

		void init() override;

		[[nodiscard]]
		Texture::IDType create(IReader&& reader, FilePathView pathHint, TextureDesc desc) override;

		[[nodiscard]]
		Texture::IDType create(const Image& image, const Array<Image>& mipmaps, TextureDesc desc) override;

		[[nodiscard]]
		Texture::IDType create(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override;

		[[nodiscard]]
		Texture::IDType create(const BCnData& bcnData) override;

		[[nodiscard]]
		Texture::IDType createDynamic(const Size& size, std::span<const Byte> data, const TextureFormat& format, TextureDesc desc) override;

		void release(Texture::IDType handleID) override;

		[[nodiscard]]
		Size getSize(Texture::IDType handleID) override;

		[[nodiscard]]
		uint32 getMipLevels(Texture::IDType handleID) override;

		[[nodiscard]]
		TextureDesc getDesc(Texture::IDType handleID) override;

		[[nodiscard]]
		TextureFormat getFormat(Texture::IDType handleID) override;

		[[nodiscard]]
		bool hasDepth(Texture::IDType handleID) override;

		bool fill(Texture::IDType handleID, const ColorF& color, bool wait) override;

		bool fill(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, bool wait) override;

		bool fillRegion(Texture::IDType handleID, const ColorF& color, const Rect& rect) override;

		bool fillRegion(Texture::IDType handleID, std::span<const Byte> src, uint32 srcBytesPerRow, const Rect& rect, bool wait) override;

		void generateMips(Texture::IDType handleID) override;

		[[nodiscard]]
		ID3D11Texture2D* getTexture(Texture::IDType handleID);

		[[nodiscard]]
		ID3D11ShaderResourceView** getSRVPtr(Texture::IDType handleID);

	private:

		CRenderer_D3D11* m_pRenderer = nullptr;

		// device のコピー
		ID3D11Device* m_device = nullptr;

		// context のコピー
		ID3D11DeviceContext* m_context = nullptr;

		// Texture の管理
		AssetHandleManager<Texture::IDType, D3D11Texture> m_textures{ "Texture" };

		std::array<bool, RenderTextureFormatCount> m_multiSampleSupportTable{};
	};
}
