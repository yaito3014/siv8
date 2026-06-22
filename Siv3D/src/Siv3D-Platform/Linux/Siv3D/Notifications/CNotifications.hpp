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
# include <Siv3D/Notifications/INotifications.hpp>

namespace s3d
{
	// TODO(linux): no desktop-notification backend yet (would use libnotify / D-Bus).
	class CNotifications final : public ISiv3DNotifications
	{
	public:

		void init() override {}

		NotificationAvailability getAvailability() override { return NotificationAvailability::Unavailable; }

		void requestPermission() override {}

		Optional<NotificationID> show(const NotificationRequest&) override { return none; }

		void dismiss(NotificationID) override {}

		void dismissAll() override {}

		Array<NotificationResponse> extractResponses() override { return{}; }
	};
}
