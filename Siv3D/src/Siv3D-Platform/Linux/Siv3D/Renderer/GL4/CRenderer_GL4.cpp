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
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Scene/SceneUtility.hpp>

namespace s3d
{
	CRenderer_GL4::~CRenderer_GL4()
	{
		LOG_SCOPED_DEBUG("CRenderer_GL4::~CRenderer_GL4()");

		if (m_captureFBO)
		{
			::glDeleteFramebuffers(1, &m_captureFBO);
			m_captureFBO = 0;
		}

		if (m_captureTexture)
		{
			::glDeleteTextures(1, &m_captureTexture);
			m_captureTexture = 0;
		}
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

		// The window framebuffer is multisampled (GLFW_SAMPLES in CWindow) to match
		// the D3D11/Metal x4 scene MSAA; enable it so 2D edges resolve smoothly.
		::glEnable(GL_MULTISAMPLE);

		LOG_INFO(fmt::format("ℹ️ OpenGL {}.{} renderer", GLVersion.major, GLVersion.minor));

		// Device strings make software rendering (e.g. GL_RENDERER "llvmpipe", from a
		// Mesa EGL/driver fallback) vs real hardware acceleration obvious in the log.
		if (const char* glVendor = reinterpret_cast<const char*>(::glGetString(GL_VENDOR)))
		{
			LOG_INFO(fmt::format("ℹ️ GL_VENDOR: {}", glVendor));
		}

		if (const char* glRenderer = reinterpret_cast<const char*>(::glGetString(GL_RENDERER)))
		{
			LOG_INFO(fmt::format("ℹ️ GL_RENDERER: {}", glRenderer));
		}

		if (const char* glVersion = reinterpret_cast<const char*>(::glGetString(GL_VERSION)))
		{
			LOG_INFO(fmt::format("ℹ️ GL_VERSION: {}", glVersion));
		}
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
		SIV3D_ENGINE(Renderer2D)->flush();

		::glFlush();
	}

	bool CRenderer_GL4::present()
	{
		// Resolve the freshly rendered back buffer into the persistent single-sample
		// capture target BEFORE the swap, so captureScreenshot() (called afterwards)
		// reads the current frame rather than the post-swap front buffer (which reads
		// back black under WSLg and is multisampled on the window).
		const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;

		if ((0 < frameBufferSize.x) && (0 < frameBufferSize.y))
		{
			updateCaptureTarget(frameBufferSize);

			::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			::glReadBuffer(GL_BACK);
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_captureFBO);
			::glBlitFramebuffer(0, 0, frameBufferSize.x, frameBufferSize.y,
								0, 0, frameBufferSize.x, frameBufferSize.y,
								GL_COLOR_BUFFER_BIT, GL_NEAREST);
			::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		::glfwSwapBuffers(m_window);
		return true;
	}

	void CRenderer_GL4::updateCaptureTarget(const Size frameBufferSize)
	{
		if ((m_captureFBO != 0) && (m_captureSize == frameBufferSize))
		{
			return;
		}

		if (m_captureTexture == 0)
		{
			::glGenTextures(1, &m_captureTexture);
		}

		::glBindTexture(GL_TEXTURE_2D, m_captureTexture);
		::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frameBufferSize.x, frameBufferSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		::glBindTexture(GL_TEXTURE_2D, 0);

		if (m_captureFBO == 0)
		{
			::glGenFramebuffers(1, &m_captureFBO);
		}

		::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_captureFBO);
		::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_captureTexture, 0);
		::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		m_captureSize = frameBufferSize;
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
		// present() already resolved the current frame into m_captureFBO.
		if ((m_captureFBO == 0) || (m_captureSize.x <= 0) || (m_captureSize.y <= 0))
		{
			m_screenCapture.clear();
			return;
		}

		const GLsizei w = m_captureSize.x;
		const GLsizei h = m_captureSize.y;

		m_screenCapture.resize(m_captureSize);

		::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_captureFBO);
		::glReadBuffer(GL_COLOR_ATTACHMENT0);
		::glPixelStorei(GL_PACK_ALIGNMENT, 1);
		::glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, m_screenCapture.data());
		::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		// GL fills rows bottom-to-top; Image is top-to-bottom. Flip in place and
		// force opaque alpha (the window backbuffer alpha is not meaningful here).
		Color* const pixels = m_screenCapture.data();

		for (int32 y = 0; y < (h / 2); ++y)
		{
			Color* const rowTop = (pixels + (static_cast<size_t>(y) * w));
			Color* const rowBottom = (pixels + (static_cast<size_t>(h - 1 - y) * w));

			for (int32 x = 0; x < w; ++x)
			{
				const Color t = rowTop[x];
				rowTop[x] = rowBottom[x];
				rowBottom[x] = t;
			}
		}

		for (Color& pixel : m_screenCapture)
		{
			pixel.a = 255;
		}
	}

	const Image& CRenderer_GL4::getScreenCapture() const
	{
		return m_screenCapture;
	}
}
