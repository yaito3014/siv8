//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CRenderer_GL4.hpp"
# include <Siv3D/Error/InternalEngineError.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Scene/SceneUtility.hpp>

namespace s3d
{
	CRenderer_GL4::~CRenderer_GL4()
	{
		LOG_SCOPED_DEBUG("CRenderer_GL4::~CRenderer_GL4()");
	}

	StringView CRenderer_GL4::getName() const
	{
		static constexpr StringView name{ U"OpenGL" };
		return name;
	}

	EngineOption::Renderer CRenderer_GL4::getRendererType() const noexcept
	{
		return EngineOption::Renderer::OpenGL;
	}

	void CRenderer_GL4::init()
	{
		LOG_SCOPED_DEBUG("CRenderer_GL4::init()");

		m_window = static_cast<GLFWwindow*>(SIV3D_ENGINE(Window)->getHandle());

		::glfwMakeContextCurrent(m_window);

		if (not ::gladLoadGLLoader(reinterpret_cast<GLADloadproc>(::glfwGetProcAddress)))
		{
			throw InternalEngineError{ "gladLoadGLLoader() failed" };
		}

		::glfwSwapInterval(m_vSyncEnabled ? 1 : 0);

		LOG_INFO(fmt::format("ℹ️ OpenGL {}.{} renderer", GLVersion.major, GLVersion.minor));
	}

	void CRenderer_GL4::waitForFrame()
	{
		// no-op
	}

	void CRenderer_GL4::beginFrame()
	{
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		::glViewport(0, 0, frameBufferSize.x, frameBufferSize.y);

		const ColorF& bg = m_sceneStyle.backgroundColor;
		::glClearColor(static_cast<float>(bg.r), static_cast<float>(bg.g), static_cast<float>(bg.b), 1.0f);
		::glClear(GL_COLOR_BUFFER_BIT);
	}

	void CRenderer_GL4::flush()
	{
		::glFlush();
	}

	bool CRenderer_GL4::present()
	{
		::glfwSwapBuffers(m_window);
		return true;
	}

	SceneStyle& CRenderer_GL4::getSceneStyle() noexcept
	{
		return m_sceneStyle;
	}

	void CRenderer_GL4::setSceneResizeMode(const ResizeMode resizeMode)
	{
		m_sceneResizeMode = resizeMode;
		updateSceneSize();
	}

	ResizeMode CRenderer_GL4::getSceneResizeMode() const noexcept
	{
		return m_sceneResizeMode;
	}

	void CRenderer_GL4::resizeSceneBuffer(const Size size)
	{
		m_sceneBufferSize = size;
	}

	const Size& CRenderer_GL4::getSceneBufferSize() const noexcept
	{
		return m_sceneBufferSize;
	}

	std::pair<double, RectF> CRenderer_GL4::getLetterboxComposition() const noexcept
	{
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		return SceneMisc::CalculateLetterboxComposition(frameBufferSize, m_sceneBufferSize);
	}

	void CRenderer_GL4::updateSceneSize()
	{
		// TODO(linux): recompute the scene buffer size for Virtual/Keep resize modes.
	}

	void CRenderer_GL4::setVSyncEnabled(const bool enabled)
	{
		m_vSyncEnabled = enabled;
		::glfwSwapInterval(m_vSyncEnabled ? 1 : 0);
	}

	bool CRenderer_GL4::isVSyncEnabled() const
	{
		return m_vSyncEnabled;
	}

	void CRenderer_GL4::captureScreenshot()
	{
		// TODO(linux): glReadPixels into m_screenCapture.
	}

	const Image& CRenderer_GL4::getScreenCapture() const
	{
		return m_screenCapture;
	}
}
