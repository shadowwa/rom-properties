/***************************************************************************
 * ROM Properties Page shell extension. (KF5)                              *
 * ExtractorPluginForwarder.hpp: KFileMetaData extractor forwarder.        *
 *                                                                         *
 * Qt's plugin system prevents a single shared library from exporting      *
 * multiple plugins, so this file acts as a KFileMetaData ExtractorPlugin, *
 * and then forwards the request to the main library.                      *
 *                                                                         *
 * Copyright (c) 2018-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_KF5_EXTRACTORPLUGINFORWARDER_HPP__
#define __ROMPROPERTIES_KDE_KF5_EXTRACTORPLUGINFORWARDER_HPP__

#include <QtCore/qglobal.h>
#include <kfilemetadata/extractorplugin.h>

namespace RomPropertiesKF5 {

class ExtractorPluginForwarder final : public ::KFileMetaData::ExtractorPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin")
	Q_INTERFACES(KFileMetaData::ExtractorPlugin)

	public:
		explicit ExtractorPluginForwarder(QObject *parent = nullptr);
		virtual ~ExtractorPluginForwarder();

	private:
		typedef KFileMetaData::ExtractorPlugin super;
		Q_DISABLE_COPY(ExtractorPluginForwarder);

	public:
		QStringList mimetypes(void) const final;
		void extract(KFileMetaData::ExtractionResult *result) final;

	private:
		// rom-properties-kf5.so handle.
		void *hRpKdeSo;

		// Actual ExtractorPlugin.
		KFileMetaData::ExtractorPlugin *fwd_plugin;

	private slots:
		/**
		 * fwd_plugin was destroyed.
		 * @param obj
		 */
		void fwd_plugin_destroyed(QObject *obj = nullptr);
};

}

#endif /* __ROMPROPERTIES_KDE_EXTRACTORPLUGINFORWARDER_HPP__ */
