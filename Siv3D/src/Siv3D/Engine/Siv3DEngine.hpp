//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2024 Ryo Suzuki
//	Copyright (c) 2016-2024 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <tuple>

# include "Siv3DComponent.hpp"

# include <Siv3D/LicenseManager/ILicenseManager.hpp>
# include <Siv3D/Logger/ILogger.hpp>
# include <Siv3D/RegExp/IRegExp.hpp>
# include <Siv3D/System/ISystem.hpp>
# include <Siv3D/EngineResource/IEngineResource.hpp>
# include <Siv3D/Profiler/IProfiler.hpp>
# include <Siv3D/UserAction/IUserAction.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/Scene/IScene.hpp>
# include <Siv3D/ImageDecoder/IImageDecoder.hpp>
# include <Siv3D/ImageEncoder/IImageEncoder.hpp>
# include <Siv3D/Emoji/IEmoji.hpp>
# include <Siv3D/Console/IConsole.hpp>
# include <Siv3D/Cursor/ICursor.hpp>
# include <Siv3D/CursorStyle/ICursorStyle.hpp>
# include <Siv3D/Mouse/IMouse.hpp>
# include <Siv3D/Keyboard/IKeyboard.hpp>
# include <Siv3D/Renderer/IRenderer.hpp>
# include <Siv3D/Shader/IShader.hpp>
# include <Siv3D/EngineShader/IEngineShader.hpp>
# include <Siv3D/Renderer2D/IRenderer2D.hpp>

namespace s3d
{
	class Siv3DEngine
	{
	private:

		inline static Siv3DEngine* pEngine = nullptr;

		std::tuple<
			Siv3DComponent<ISiv3DLicenseManager>,
			Siv3DComponent<ISiv3DLogger>,
			Siv3DComponent<ISiv3DRegExp>,
			Siv3DComponent<ISiv3DSystem>,
			Siv3DComponent<ISiv3DEngineResource>,
			Siv3DComponent<ISiv3DProfiler>,
			Siv3DComponent<ISiv3DUserAction>,
			Siv3DComponent<ISiv3DWindow>,
			Siv3DComponent<ISiv3DScene>,
			Siv3DComponent<ISiv3DImageDecoder>,
			Siv3DComponent<ISiv3DImageEncoder>,
			Siv3DComponent<ISiv3DEmoji>,
			Siv3DComponent<ISiv3DConsole>,
			Siv3DComponent<ISiv3DCursor>,
			Siv3DComponent<ISiv3DCursorStyle>,
			Siv3DComponent<ISiv3DKeyboard>,
			Siv3DComponent<ISiv3DMouse>,
			Siv3DComponent<ISiv3DRenderer>,
			Siv3DComponent<ISiv3DShader>,
			Siv3DComponent<ISiv3DEngineShader>,
			Siv3DComponent<ISiv3DRenderer2D>
		> m_components;

	public:

		Siv3DEngine() noexcept;

		~Siv3DEngine();

		[[nodiscard]]
		static bool isNull() noexcept;

		[[nodiscard]]
		static bool isAvailable() noexcept;

		template <class Interface>
		[[nodiscard]]
		static auto* Get() noexcept
		{
			return std::get<Siv3DComponent<Interface>>(pEngine->m_components).get();
		}
	};

	# define SIV3D_ENGINE(COMPONENT) Siv3DEngine::Get<ISiv3D##COMPONENT>()
}
