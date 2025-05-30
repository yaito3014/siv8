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
	//	MakeInt128
	//
	////////////////////////////////////////////////////////////////

	constexpr int128 MakeInt128(const int64 high, const uint64 low) noexcept
	{
		return absl::MakeInt128(high, low);
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	MakeUint128
	//
	////////////////////////////////////////////////////////////////

	constexpr uint128 MakeUint128(const uint64 high, const uint64 low) noexcept
	{
		return absl::MakeUint128(high, low);
	}
}
