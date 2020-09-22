/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * OverlayIconPlugin.hpp: KOverlayIconPlugin.                              *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KOverlayIconPlugin,            *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_KF5_OVERLAYICONPLUGIN_HPP__
#define __ROMPROPERTIES_KDE_KF5_OVERLAYICONPLUGIN_HPP__

#include <QtCore/qglobal.h>
#include <KOverlayIconPlugin>

namespace RomPropertiesKF5 {

class OverlayIconPlugin final : public ::KOverlayIconPlugin
{
	Q_OBJECT
	//Q_INTERFACES(KOverlayIconPlugin)

	public:
		explicit OverlayIconPlugin(QObject *parent = nullptr);

	private:
		typedef KOverlayIconPlugin super;
		Q_DISABLE_COPY(OverlayIconPlugin);

	public:
		QStringList getOverlays(const QUrl &item) final;
};

// Exported function pointer to create a new RpExtractorPlugin.
typedef OverlayIconPlugin* (*pfn_createOverlayIconPluginKDE_t)(QObject *parent);
#define PFN_CREATEOVERLAYICONPLUGINKDE_FN createOverlayIconPluginKF5
#define PFN_CREATEOVERLAYICONPLUGINKDE_NAME "createOverlayIconPluginKF5"

}

#endif /* __ROMPROPERTIES_KDE_KF5_OVERLAYICONPLUGIN_HPP__ */
