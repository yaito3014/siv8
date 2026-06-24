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
# include <Siv3D/Renderer/IRenderer.hpp>
# include <Siv3D/ResizeMode.hpp>
# include <Siv3D/Image.hpp>
# include <Siv3D/Scene.hpp>
# include <Siv3D/Scene/SceneStyle.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	class CRenderer_GL4 final : public ISiv3DRenderer
	{
	public:

		~CRenderer_GL4() override;

		StringView getName() const override;

		EngineOption::Renderer getRendererType() const noexcept override;

		void init() override;

		void waitForFrame() override;

		void beginFrame() override;

		void flush() override;

		bool present() override;

		SceneStyle& getSceneStyle() noexcept override;

		void setSceneResizeMode(ResizeMode resizeMode) override;

		ResizeMode getSceneResizeMode() const noexcept override;

		void resizeSceneBuffer(Size size) override;

		const Size& getSceneBufferSize() const noexcept override;

		std::pair<double, RectF> getLetterboxComposition() const noexcept override;

		void updateSceneSize() override;

		void setVSyncEnabled(bool enabled) override;

		bool isVSyncEnabled() const override;

		void captureScreenshot() override;

		const Image& getScreenCapture() const override;

	private:

		// Resolve the back buffer into m_captureFBO before each swap, so
		// captureScreenshot() (invoked after present()) can read the just-rendered
		// frame. Reading the FRONT buffer after the swap returns black under some
		// compositors (e.g. WSLg), and the window framebuffer is multisampled, so a
		// persistent single-sample resolve target is the portable option.
		void updateCaptureTarget(Size frameBufferSize);

		GLFWwindow* m_window = nullptr;

		SceneStyle m_sceneStyle;

		ResizeMode m_sceneResizeMode = ResizeMode::Virtual;

		Size m_sceneBufferSize = Scene::DefaultSceneSize;

		bool m_vSyncEnabled = true;

		Image m_screenCapture;

		// Persistent single-sample resolve target for screenshots (see captureScreenshot).
		GLuint m_captureFBO = 0;

		GLuint m_captureTexture = 0;

		Size m_captureSize{ 0, 0 };
	};
}
