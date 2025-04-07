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
# include <cassert>

#include "Siv3D/LicenseManager/ILicenseManager.hpp"
#include "Siv3D/Logger/ILogger.hpp"
#include "Siv3D/RegExp/IRegExp.hpp"
#include "Siv3D/System/ISystem.hpp"
#include "Siv3D/EngineResource/IEngineResource.hpp"
#include "Siv3D/Profiler/IProfiler.hpp"
#include "Siv3D/AssetMonitor/IAssetMonitor.hpp"
#include "Siv3D/UserAction/IUserAction.hpp"
#include "Siv3D/Window/IWindow.hpp"
#include "Siv3D/Scene/IScene.hpp"
#include "Siv3D/ImageDecoder/IImageDecoder.hpp"
#include "Siv3D/ImageEncoder/IImageEncoder.hpp"
#include "Siv3D/Emoji/IEmoji.hpp"
#include "Siv3D/Console/IConsole.hpp"
#include "Siv3D/Cursor/ICursor.hpp"
#include "Siv3D/CursorStyle/ICursorStyle.hpp"
#include "Siv3D/Keyboard/IKeyboard.hpp"
#include "Siv3D/Mouse/IMouse.hpp"
#include "Siv3D/Renderer/IRenderer.hpp"
#include "Siv3D/Texture/ITexture.hpp"
#include "Siv3D/Shader/IShader.hpp"
#include "Siv3D/EngineShader/IEngineShader.hpp"
#include "Siv3D/Renderer2D/IRenderer2D.hpp"
#include "Siv3D/ScreenCapture/IScreenCapture.hpp"

namespace s3d
{
	template <class Interface>
	class Siv3DComponent
	{
	public:

		~Siv3DComponent()
		{
			// must be released prior to the destructor executing
			assert(pInterface == nullptr);
		}

		[[nodiscard]]
		Interface* get() noexcept
		{
			return pInterface;
		}

		void release()
		{
			delete pInterface;
			pInterface = nullptr;
		}

	private:

		Interface* pInterface = Interface::Create();
	};
}
