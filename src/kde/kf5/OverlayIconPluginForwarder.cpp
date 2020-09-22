/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * OverlayIconPluginForwarder.cpp: KOverlayIconPlugin forwarder.           *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KOverlayIconPlugin,            *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "config.kf5.h"

#include "OverlayIconPluginForwarder.hpp"
#include "OverlayIconPlugin.hpp"
#include "../RpQt.hpp"

// C includes.
#include <dlfcn.h>

// KDE includes.
#include <kfileitem.h>
#include <KOverlayIconPlugin>

#ifndef KF5_PRPD_PLUGIN_INSTALL_DIR
#  error KF5_PRPD_PLUGIN_INSTALL_DIR is not set.
#endif /* KF5_PRPD_PLUGIN_INSTALL_DIR */

namespace RomPropertiesKF5 {

OverlayIconPluginForwarder::OverlayIconPluginForwarder(QObject *parent)
	: super(parent)
	, hRpKdeSo(nullptr)
	, fwd_plugin(nullptr)
{
	if (getuid() == 0 || geteuid() == 0) {
		qCritical("*** overlayiconplugin_rom_properties_" RP_KDE_LOWER "%u does not support running as root.", QT_VERSION >> 16);
		return;
	}

	// FIXME: Check the .desktop file?
	QString pluginPath(QString::fromUtf8(KF5_PLUGIN_INSTALL_DIR));
	pluginPath += QLatin1String("/rom-properties-kf5.so");

	// Attempt to load the plugin.
	hRpKdeSo = dlopen(pluginPath.toUtf8().constData(), RTLD_LOCAL|RTLD_LAZY);
	if (!hRpKdeSo) {
		// Unable to open the plugin.
		// NOTE: We can't use mismatched plugins here.
		return;
	}

	// Load the symbol.
	pfn_createOverlayIconPluginKDE_t pfn = reinterpret_cast<pfn_createOverlayIconPluginKDE_t>(
		dlsym(hRpKdeSo, PFN_CREATEOVERLAYICONPLUGINKDE_NAME));
	if (!pfn) {
		// Symbol not found.
		dlclose(hRpKdeSo);
		hRpKdeSo = nullptr;
		return;
	}

	// Create an OverlayIconPlugin object.
	fwd_plugin = pfn(this);
	if (!fwd_plugin) {
		// Unable to create an OverlayIconPlugin object.
		dlclose(hRpKdeSo);
		hRpKdeSo = nullptr;
		return;
	}

	// Make sure we know if the OverlayPlugin gets deleted.
	// This *shouldn't* happen, but it's possible that our parent
	// object enumerates child objects and does weird things.
	connect(fwd_plugin, &QObject::destroyed,
		this, &OverlayIconPluginForwarder::fwd_plugin_destroyed);
}

OverlayIconPluginForwarder::~OverlayIconPluginForwarder()
{
	delete fwd_plugin;

	// NOTE: dlclose(nullptr) may crash, so we have to check for nullptr.
	if (hRpKdeSo) {
		dlclose(hRpKdeSo);
	}
}

QStringList OverlayIconPluginForwarder::getOverlays(const QUrl &item)
{
	if (fwd_plugin) {
		return fwd_plugin->getOverlays(item);
	}
	return QStringList();
}

/**
 * fwd_plugin was destroyed.
 * @param obj
 */
void OverlayIconPluginForwarder::fwd_plugin_destroyed(QObject *obj)
{
	if (obj == fwd_plugin) {
		// Object matches.
		// NULL it out so we don't have problems later.
		fwd_plugin = nullptr;
	}
}

}
