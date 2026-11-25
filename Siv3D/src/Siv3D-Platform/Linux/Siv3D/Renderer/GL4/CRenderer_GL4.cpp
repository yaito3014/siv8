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
# include <algorithm>
# include <cmath>
# include <Siv3D/Error/InternalEngineError.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Shader/IShader.hpp>
# include <Siv3D/Texture/ITexture.hpp>
# include <Siv3D/EngineShader/IEngineShader.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Scene/SceneUtility.hpp>

namespace s3d
{
	namespace
	{
		static void ClearBoundFramebuffer(const Size& size, const ColorF& color)
		{
			::glViewport(0, 0, size.x, size.y);
			::glClearColor(static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b), static_cast<float>(color.a));
			::glClear(GL_COLOR_BUFFER_BIT);
		}

		static void CheckFramebuffer(const char* name)
		{
			if (::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				throw InternalEngineError{ fmt::format("CRenderer_GL4: the {} framebuffer is incomplete", name) };
			}
		}
	}

	CRenderer_GL4::~CRenderer_GL4()
	{
		LOG_SCOPED_DEBUG("CRenderer_GL4::~CRenderer_GL4()");

		destroySceneBuffers();
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

		// The GL context is now current; bring up the texture and shader subsystems
		// (mirrors CRenderer_Metal). CShader_GL4 sets up the program pipeline;
		// CEngineShader_GL4 compiles the built-in 2D GLSL shaders that CRenderer2D_GL4 binds.
		SIV3D_ENGINE(Texture)->init();
		SIV3D_ENGINE(Shader)->init();
		SIV3D_ENGINE(EngineShader)->init();

		{
			GLint maxSamples = 1;
			::glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
			m_scene.sampleCount = Clamp<uint32>(m_scene.sampleCount, 1, static_cast<uint32>(Max(maxSamples, 1)));
			LOG_INFO(fmt::format("ℹ️ Scene MSAA: x{}", m_scene.sampleCount));
		}

		m_backBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
		createSceneBuffers(SIV3D_ENGINE(Window)->getState().virtualSize);
	}

	void CRenderer_GL4::waitForFrame()
	{
		// no-op
	}

	void CRenderer_GL4::beginFrame()
	{
		if (const Size frameBufferSize = SIV3D_ENGINE(Window)->getState().frameBufferSize;
			(frameBufferSize != m_backBufferSize) && (0 < frameBufferSize.x) && (0 < frameBufferSize.y))
		{
			LOG_DEBUG(fmt::format("CRenderer_GL4: back buffer resized to {}", frameBufferSize));
			m_backBufferSize = frameBufferSize;
			updateSceneBufferSize();
		}

		clearScene();

		::glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const ColorF& letterbox = m_sceneStyle.letterboxColor;
		ClearBoundFramebuffer(m_backBufferSize, ColorF{ letterbox.r, letterbox.g, letterbox.b, 1.0 });
	}

	void CRenderer_GL4::flush()
	{
		SIV3D_ENGINE(Renderer2D)->flush();

		renderSceneToBackBuffer();

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
		updateSceneBufferSize();
	}

	ResizeMode CRenderer_GL4::getSceneResizeMode() const noexcept
	{
		return m_sceneResizeMode;
	}

	void CRenderer_GL4::resizeSceneBuffer(const Size size)
	{
		if ((size.x <= 0) || (size.y <= 0) || (size == m_scene.size))
		{
			return;
		}

		LOG_DEBUG(fmt::format("CRenderer_GL4::resizeSceneBuffer({})", size));

		destroySceneBuffers();
		createSceneBuffers(size);
	}

	const Size& CRenderer_GL4::getSceneBufferSize() const noexcept
	{
		return m_scene.size;
	}

	std::pair<double, RectF> CRenderer_GL4::getLetterboxComposition() const noexcept
	{
		return SceneMisc::CalculateLetterboxComposition(m_backBufferSize, m_scene.size);
	}

	void CRenderer_GL4::updateSceneSize()
	{
		updateSceneBufferSize();
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
		resolveScene();

		const Size size = m_scene.size;
		m_screenCapture.resize(size);

		::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_scene.framebuffer);
		::glReadBuffer(GL_COLOR_ATTACHMENT0);
		::glPixelStorei(GL_PACK_ALIGNMENT, 4);
		::glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, m_screenCapture.data());
		::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		// GL returns rows bottom-to-top, Image is top-to-bottom; the capture is opaque
		// like the D3D11/Metal ones.
		Color* const pixels = m_screenCapture.data();

		for (int32 y = 0; y < (size.y / 2); ++y)
		{
			std::swap_ranges((pixels + (static_cast<size_t>(y) * size.x)), (pixels + (static_cast<size_t>(y + 1) * size.x)),
				(pixels + (static_cast<size_t>(size.y - 1 - y) * size.x)));
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

	void CRenderer_GL4::bindSceneFramebuffer() const
	{
		::glBindFramebuffer(GL_FRAMEBUFFER, ((1 < m_scene.sampleCount) ? m_scene.msaaFramebuffer : m_scene.framebuffer));
	}

	void CRenderer_GL4::createSceneBuffers(const Size size)
	{
		m_scene.size = size;
		m_scene.resolved = false;

		::glGenTextures(1, &m_scene.texture);
		::glBindTexture(GL_TEXTURE_2D, m_scene.texture);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		::glBindTexture(GL_TEXTURE_2D, 0);

		::glGenFramebuffers(1, &m_scene.framebuffer);
		::glBindFramebuffer(GL_FRAMEBUFFER, m_scene.framebuffer);
		::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_scene.texture, 0);
		CheckFramebuffer("scene");

		if (1 < m_scene.sampleCount)
		{
			::glGenRenderbuffers(1, &m_scene.msaaRenderbuffer);
			::glBindRenderbuffer(GL_RENDERBUFFER, m_scene.msaaRenderbuffer);
			::glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(m_scene.sampleCount), GL_RGBA8, size.x, size.y);
			::glBindRenderbuffer(GL_RENDERBUFFER, 0);

			::glGenFramebuffers(1, &m_scene.msaaFramebuffer);
			::glBindFramebuffer(GL_FRAMEBUFFER, m_scene.msaaFramebuffer);
			::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_scene.msaaRenderbuffer);
			CheckFramebuffer("MSAA scene");
		}

		::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		clearScene();
	}

	void CRenderer_GL4::destroySceneBuffers()
	{
		if (m_scene.msaaFramebuffer)
		{
			::glDeleteFramebuffers(1, &m_scene.msaaFramebuffer);
		}

		if (m_scene.msaaRenderbuffer)
		{
			::glDeleteRenderbuffers(1, &m_scene.msaaRenderbuffer);
		}

		if (m_scene.framebuffer)
		{
			::glDeleteFramebuffers(1, &m_scene.framebuffer);
		}

		if (m_scene.texture)
		{
			::glDeleteTextures(1, &m_scene.texture);
		}

		m_scene = SceneBuffers{ .sampleCount = m_scene.sampleCount };
	}

	void CRenderer_GL4::clearScene()
	{
		bindSceneFramebuffer();
		ClearBoundFramebuffer(m_scene.size, m_sceneStyle.backgroundColor);
		::glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_scene.resolved = false;
	}

	void CRenderer_GL4::resolveScene()
	{
		if ((m_scene.sampleCount <= 1) || m_scene.resolved)
		{
			return;
		}

		const Size size = m_scene.size;
		::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_scene.msaaFramebuffer);
		::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_scene.framebuffer);
		::glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		::glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_scene.resolved = true;
	}

	void CRenderer_GL4::renderSceneToBackBuffer()
	{
		if ((m_backBufferSize.x <= 0) || (m_backBufferSize.y <= 0))
		{
			return;
		}

		const Size sceneSize = m_scene.size;

		// Same size: a multisampled scene resolves straight into the window, as on D3D11.
		if ((1 < m_scene.sampleCount) && (m_backBufferSize == sceneSize) && (not m_scene.resolved))
		{
			::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_scene.msaaFramebuffer);
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			::glBlitFramebuffer(0, 0, sceneSize.x, sceneSize.y, 0, 0, sceneSize.x, sceneSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			::glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		resolveScene();

		// Letterbox rect in window space, whose origin is the bottom-left corner.
		const auto [s, viewRect] = getLetterboxComposition();
		const GLint x0 = static_cast<GLint>(std::lround(viewRect.x));
		const GLint x1 = static_cast<GLint>(std::lround(viewRect.x + viewRect.w));
		const GLint y0 = static_cast<GLint>(std::lround(m_backBufferSize.y - (viewRect.y + viewRect.h)));
		const GLint y1 = static_cast<GLint>(std::lround(m_backBufferSize.y - viewRect.y));
		const GLenum filter = ((m_sceneStyle.textureFilter == TextureFilter::Linear) ? GL_LINEAR : GL_NEAREST);

		::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_scene.framebuffer);
		::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		::glBlitFramebuffer(0, 0, sceneSize.x, sceneSize.y, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, filter);
		::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CRenderer_GL4::updateSceneBufferSize()
	{
		if (m_sceneResizeMode == ResizeMode::Actual)
		{
			resizeSceneBuffer(SIV3D_ENGINE(Window)->getState().frameBufferSize);
		}
		else if (m_sceneResizeMode == ResizeMode::Virtual)
		{
			resizeSceneBuffer(SIV3D_ENGINE(Window)->getState().virtualSize);
		}
	}
}
