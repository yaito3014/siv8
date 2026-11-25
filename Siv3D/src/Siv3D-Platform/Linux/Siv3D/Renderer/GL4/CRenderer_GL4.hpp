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

		/// @brief Binds the framebuffer the 2D renderer draws the scene into.
		void bindSceneFramebuffer() const;

	private:

		// The scene is drawn into `msaaFramebuffer` (or `framebuffer` without MSAA), then
		// resolved into `texture` / `framebuffer`, which is letterboxed onto the window
		// and read for screenshots.
		struct SceneBuffers
		{
			uint32 sampleCount = FromEnum(Scene::DefaultMSAASampleCount);

			Size size{ 0, 0 };

			GLuint msaaFramebuffer = 0;

			GLuint msaaRenderbuffer = 0;

			GLuint framebuffer = 0;

			GLuint texture = 0;

			// Whether `texture` holds the current frame of a multisampled scene.
			bool resolved = false;
		};

		void createSceneBuffers(Size size);

		void destroySceneBuffers();

		void clearScene();

		void resolveScene();

		void renderSceneToBackBuffer();

		void updateSceneBufferSize();

		GLFWwindow* m_window = nullptr;

		SceneStyle m_sceneStyle;

		ResizeMode m_sceneResizeMode = Scene::DefaultResizeMode;

		SceneBuffers m_scene;

		Size m_backBufferSize{ 0, 0 };

		bool m_vSyncEnabled = true;

		Image m_screenCapture;
	};
}
