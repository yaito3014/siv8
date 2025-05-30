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

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	clearInput
	//
	////////////////////////////////////////////////////////////////

	inline void InputState::clearInput()
	{
		m_cleared = true;
	}

	////////////////////////////////////////////////////////////////
	//
	//	isCleared
	//
	////////////////////////////////////////////////////////////////

	inline bool InputState::isCleared() const noexcept
	{
		return m_cleared;
	}

	////////////////////////////////////////////////////////////////
	//
	//	up
	//
	////////////////////////////////////////////////////////////////

	inline bool InputState::up() const noexcept
	{
		return m_up;
	}

	////////////////////////////////////////////////////////////////
	//
	//	pressed
	//
	////////////////////////////////////////////////////////////////

	inline bool InputState::pressed() const noexcept
	{
		return ((not m_cleared) && m_pressed);
	}

	////////////////////////////////////////////////////////////////
	//
	//	down
	//
	////////////////////////////////////////////////////////////////

	inline bool InputState::down() const noexcept
	{
		return ((not m_cleared) && m_down);
	}
}
