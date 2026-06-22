//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2026 Ryo Suzuki
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "CCursor.hpp"
# include <Siv3D/Time.hpp>
# include <Siv3D/Math.hpp>
# include <Siv3D/WindowState.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/Window/IWindow.hpp>
# include <Siv3D/Renderer/IRenderer.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>

namespace s3d
{
	namespace
	{
		// GLFW reports the cursor position in content-area (client) coordinates.
		[[nodiscard]]
		static Point GetCursorRawClientPos(GLFWwindow* window)
		{
			double x = 0.0, y = 0.0;
			::glfwGetCursorPos(window, &x, &y);
			return Math::Round(Vec2{ x, y }).asPoint();
		}

		[[nodiscard]]
		static Point ToScreenPos(const Point rawClientPos)
		{
			const WindowState windowState = SIV3D_ENGINE(Window)->getState();
			return (rawClientPos + windowState.bounds.pos + Point{ 0, windowState.titleBarHeight });
		}
	}

	CCursor::~CCursor()
	{
		LOG_SCOPED_DEBUG("CCursor::~CCursor()");
	}

	void CCursor::init()
	{
		LOG_SCOPED_DEBUG("CCursor::init()");

		m_window = static_cast<GLFWwindow*>(SIV3D_ENGINE(Window)->getHandle());

		{
			const Point rawClientPos = GetCursorRawClientPos(m_window);
			const Point screenPos = ToScreenPos(rawClientPos);
			updateHighTemporalResolutionCursorPos(rawClientPos);

			const double windowScaling = SIV3D_ENGINE(Window)->getState().scaling;
			const Vec2 clientPos = (rawClientPos / windowScaling);
			m_state.update(screenPos, rawClientPos, clientPos);
		}
	}

	void CCursor::updateHighTemporalResolutionCursorPos(const Point rawClientPos)
	{
		m_highTemporalResolutionCursor.add(rawClientPos);
	}

	void CCursor::update()
	{
		if (m_clippedToWindow)
		{
			// TODO(linux): clip cursor to the window region.
		}

		m_highTemporalResolutionCursor.update();

		{
			m_transform.setBaseWindow(SIV3D_ENGINE(Renderer)->getLetterboxComposition());
		}

		{
			const Point rawClientPos = GetCursorRawClientPos(m_window);
			const Point screenPos = ToScreenPos(rawClientPos);
			updateHighTemporalResolutionCursorPos(rawClientPos);

			const Vec2 clientPos = m_transform.allInv.transformPoint(rawClientPos);
			m_state.update(screenPos, rawClientPos, clientPos);
		}

		m_captured = false;
	}

	const CursorState& CCursor::getState() const
	{
		return m_state;
	}

	Array<std::pair<int64, Point>> CCursor::getHighTemporalResolutionCursorPos() const
	{
		return m_highTemporalResolutionCursor.get();
	}

	void CCursor::setPos(const Point pos)
	{
		const Vec2 rawPos = m_transform.all.transformPoint(pos);

		::glfwSetCursorPos(m_window, rawPos.x, rawPos.y);
	}

	const Mat3x2& CCursor::getBaseWindowTransform() const noexcept
	{
		return m_transform.baseWindow;
	}

	const Mat3x2& CCursor::getCameraTransform() const noexcept
	{
		return m_transform.camera;
	}

	const Mat3x2& CCursor::getLocalTransform() const noexcept
	{
		return m_transform.local;
	}

	void CCursor::setCameraTransform(const Mat3x2& matrix)
	{
		if (m_transform.camera == matrix)
		{
			return;
		}

		m_transform.setCamera(matrix);

		m_state.vec2.previous	= m_transform.allInv.transformPoint(m_state.raw.previous);
		m_state.vec2.current	= m_transform.allInv.transformPoint(m_state.raw.current);
		m_state.vec2.delta		= (m_state.vec2.current - m_state.vec2.previous);

		m_state.point.previous	= m_state.vec2.previous.asPoint();
		m_state.point.current	= m_state.vec2.current.asPoint();
		m_state.point.delta		= m_state.vec2.delta.asPoint();
	}

	void CCursor::setLocalTransform(const Mat3x2& matrix)
	{
		if (m_transform.local == matrix)
		{
			return;
		}

		m_transform.setLocal(matrix);

		m_state.vec2.previous	= m_transform.allInv.transformPoint(m_state.raw.previous);
		m_state.vec2.current	= m_transform.allInv.transformPoint(m_state.raw.current);
		m_state.vec2.delta		= (m_state.vec2.current - m_state.vec2.previous);

		m_state.point.previous	= m_state.vec2.previous.asPoint();
		m_state.point.current	= m_state.vec2.current.asPoint();
		m_state.point.delta		= m_state.vec2.delta.asPoint();
	}

	bool CCursor::isClippedToWindow() const noexcept
	{
		return m_clippedToWindow;
	}

	void CCursor::clipToWindow(const bool clip)
	{
		if (clip == m_clippedToWindow)
		{
			return;
		}

		m_clippedToWindow = clip;
	}

	void CCursor::setCapture(const bool captured) noexcept
	{
		m_captured = captured;
	}

	bool CCursor::isCaptured() const noexcept
	{
		return m_captured;
	}
}
