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
# include <Siv3D/Pentablet/IPentablet.hpp>
# include <Siv3D/PenCaps.hpp>
# include <Siv3D/PenState.hpp>
# include <Siv3D/String.hpp>

namespace s3d
{
	// TODO(linux): no pen-tablet backend yet.
	class CPentablet final : public ISiv3DPentablet
	{
	public:

		void initLibrary() override {}

		void resetDevice() override {}

		void update() override {}

		void onProximity(bool, const Optional<bool>&) override {}

		void onPenMove(double, double, double, double) override {}

		bool isAvailable() override { return false; }

		bool isConnected() override { return false; }

		const String& getName() override { static const String name; return name; }

		const PenCaps& getCaps() override { static const PenCaps caps{}; return caps; }

		const PenState& getState() override { static const PenState state{}; return state; }
	};
}
