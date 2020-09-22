/***************************************************************************
 * ROM Properties Page shell extension. (KF5)                              *
 * ExtractorPluginForwarder.cpp: KFileMetaData extractor forwarder.        *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KFileMetaData ExtractorPlugin, *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "config.kf5.h"

#include "ExtractorPluginForwarder.hpp"
#include "ExtractorPlugin.hpp"
#include "../RpQt.hpp"

// C includes.
#include <dlfcn.h>

// KDE includes.
#define SO_FILENAME "rom-properties-kf5.so"
#include <kfileitem.h>
#include <kfilemetadata/extractorplugin.h>
using KFileMetaData::ExtractorPlugin;
using KFileMetaData::ExtractionResult;

#ifndef KF5_PRPD_PLUGIN_INSTALL_DIR
#  error KF5_PRPD_PLUGIN_INSTALL_DIR is not set.
#endif

namespace RomPropertiesKF5 {

ExtractorPluginForwarder::ExtractorPluginForwarder(QObject *parent)
	: super(parent)
	, hRpKdeSo(nullptr)
	, fwd_plugin(nullptr)
{
	if (getuid() == 0 || geteuid() == 0) {
		qCritical("*** kfilemetadata_rom_properties_" RP_KDE_LOWER "%u does not support running as root.", QT_VERSION >> 16);
		return;
	}

	// FIXME: Check the .desktop file?
	QString pluginPath(QString::fromUtf8(KF5_PLUGIN_INSTALL_DIR));
	pluginPath += QLatin1String("/" SO_FILENAME);

	// Attempt to load the plugin.
	hRpKdeSo = dlopen(pluginPath.toUtf8().constData(), RTLD_LOCAL|RTLD_LAZY);
	if (!hRpKdeSo) {
		// Unable to open the plugin.
		// NOTE: We can't use mismatched plugins here.
		return;
	}

	// Load the symbol.
	pfn_createExtractorPluginKDE_t pfn = reinterpret_cast<pfn_createExtractorPluginKDE_t>(
		dlsym(hRpKdeSo, PFN_CREATEEXTRACTORPLUGINKDE_NAME));
	if (!pfn) {
		// Symbol not found.
		dlclose(hRpKdeSo);
		hRpKdeSo = nullptr;
		return;
	}

	// Create an ExtractorPlugin object.
	fwd_plugin = pfn(this);
	if (!fwd_plugin) {
		// Unable to create an ExtractorPlugin object.
		dlclose(hRpKdeSo);
		hRpKdeSo = nullptr;
		return;
	}

	// Make sure we know if the ExtractorPlugin gets deleted.
	// This *shouldn't* happen, but it's possible that our parent
	// object enumerates child objects and does weird things.
	connect(fwd_plugin, &QObject::destroyed,
		this, &ExtractorPluginForwarder::fwd_plugin_destroyed);
}

ExtractorPluginForwarder::~ExtractorPluginForwarder()
{
	delete fwd_plugin;

	// NOTE: dlclose(nullptr) may crash, so we have to check for nullptr.
	if (hRpKdeSo) {
		dlclose(hRpKdeSo);
	}
}

QStringList ExtractorPluginForwarder::mimetypes(void) const
{
	if (fwd_plugin) {
		return fwd_plugin->mimetypes();
	}
	return QStringList();
}

void ExtractorPluginForwarder::extract(ExtractionResult *result)
{
	if (fwd_plugin) {
		fwd_plugin->extract(result);
	}
}

/**
 * fwd_plugin was destroyed.
 * @param obj
 */
void ExtractorPluginForwarder::fwd_plugin_destroyed(QObject *obj)
{
	if (obj == fwd_plugin) {
		// Object matches.
		// NULL it out so we don't have problems later.
		fwd_plugin = nullptr;
	}
}

}
