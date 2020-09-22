/***************************************************************************
 * ROM Properties Page shell extension. (GTK+ common)                      *
 * MessageSound.cpp: Message sound effects class.                          *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "MessageSound.hpp"

#include <canberra.h>
#include <canberra-gtk.h>

/**
 * Play a message sound effect.
 * @param notificationType Notification type.
 * @param message Message for logging.
 * @param parent Parent window.
 */
void MessageSound::play(GtkMessageType notificationType, const char *message, GtkWidget *parent)
{
	// Check if event sounds are enabled.
	GtkSettings *const settings = gtk_settings_get_default();
	gboolean bEnableEventSounds = false;
	g_object_get(settings, "gtk-enable-event-sounds", &bEnableEventSounds, nullptr);
	if (!bEnableEventSounds) {
		// Event sounds are disabled.
		return;
	}

	const char *event_id;
	switch (notificationType) {
		default:
		case GTK_MESSAGE_INFO:
		case GTK_MESSAGE_QUESTION:
		case GTK_MESSAGE_OTHER:
			event_id = "dialog-information";
			break;
		case GTK_MESSAGE_WARNING:
			event_id = "dialog-warning";
			break;
		case GTK_MESSAGE_ERROR:
			event_id = "dialog-error";
			break;
	}

	if (parent) {
		// NOTE: Description cannot be nullptr. Otherwise, the sound won't play.
		ca_gtk_play_for_widget(gtk_widget_get_toplevel(parent), 0,
			CA_PROP_EVENT_ID, event_id,
			CA_PROP_EVENT_DESCRIPTION,
				((message && message[0] != '\0') ? message : ""),
			nullptr);
	} else {
		ca_context *const ctx = ca_gtk_context_get();
		if (!ctx)
			return;

		ca_context_play(ctx, 0,
			CA_PROP_EVENT_ID, event_id,
			CA_PROP_EVENT_DESCRIPTION,
				((message && message[0] != '\0') ? message : ""),
			nullptr);
	}
}
