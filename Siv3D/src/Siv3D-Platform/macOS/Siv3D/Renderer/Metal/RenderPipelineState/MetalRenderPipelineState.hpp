//-----------------------------------------------
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
# include <Siv3D/HashMap.hpp>
# include <Siv3D/Renderer/Metal/Metal.hpp>
# include "PipelineStateDesc.hpp"

namespace s3d
{
	class CShader_Metal;

	class MetalRenderPipelineState
	{
	public:

		void init(MTL::Device* device, CShader_Metal* pShader);
		
		const MTL::RenderPipelineState* get(const PipelineStateDesc& desc);

	private:
	
		MTL::Device* m_device			= nullptr;

		CShader_Metal* m_pShader		= nullptr;

		HashMap<PipelineStateDesc, NS::SharedPtr<MTL::RenderPipelineState>> m_pipelineStates;
	};
}
