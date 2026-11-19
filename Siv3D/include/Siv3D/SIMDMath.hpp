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

# pragma once
# include "Common.hpp"

// Not on GCC: DirectXMath's SSE4 XMStoreFloat3 writes float members through
// int* (an aliasing violation), and GCC's type-based alias analysis drops or
// reorders those stores at -O2 and above. Its SSE2 path is used there instead.
# if SIV3D_INTRINSIC(SSE) && !SIV3D_COMPILER(GCC)
#	define _XM_SSE4_INTRINSICS_
# endif

# ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdefaulted-function-deleted"
# endif

# include <DirectXMath.h>
# include <DirectXPackedVector.h>

# ifdef __clang__
#	pragma clang diagnostic pop
# endif

# if !SIV3D_PLATFORM(WINDOWS)
#	undef __in
#	undef __out
#	undef __valid
# endif

namespace s3d
{
    struct FormatData;

    ////////////////////////////////////////////////////////////////
    //
    //	aligned_float4
    //
    ////////////////////////////////////////////////////////////////

	using aligned_float4 = DirectX::XMVECTOR;

# define SIV3D_VECTOR_CALL XM_CALLCONV

    ////////////////////////////////////////////////////////////////
    //
    //	Formatter
    //
    ////////////////////////////////////////////////////////////////

    void Formatter(FormatData& formatData, aligned_float4 value);
}
