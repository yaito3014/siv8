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
# include <Siv3D/TextToSpeech/ITextToSpeech.hpp>

namespace s3d
{
	// TODO(linux): no speech-synthesis backend yet (would use speech-dispatcher).
	class CTextToSpeech final : public ISiv3DTextToSpeech
	{
	public:

		void init() override {}

		bool synthesizeToWave(StringView, Wave&) override { return false; }
	};
}
