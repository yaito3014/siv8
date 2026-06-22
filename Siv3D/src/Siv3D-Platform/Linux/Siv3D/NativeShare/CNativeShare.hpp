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
# include <Siv3D/NativeShare/INativeShare.hpp>

namespace s3d
{
	// TODO(linux): no native share-sheet backend yet.
	class CNativeShare final : public ISiv3DNativeShare
	{
	public:

		void init() override {}

		bool show(const Image&) override { return false; }
	};
}
