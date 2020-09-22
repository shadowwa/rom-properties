/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * KeyStoreQt.hpp: Key store object for Qt.                                *
 *                                                                         *
 * Copyright (c) 2012-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_CONFIG_KEYSTOREQT_HPP__
#define __ROMPROPERTIES_KDE_CONFIG_KEYSTOREQT_HPP__

#include "libromdata/crypto/KeyStoreUI.hpp"
#include <QtCore/QObject>

class KeyStoreQt : public QObject, public LibRomData::KeyStoreUI
{
	Q_OBJECT

	Q_PROPERTY(int totalKeyCount READ totalKeyCount)
	// NOTE: modified() isn't a modifier, since it's only emitted
	// if d->changed becomes true.
	Q_PROPERTY(bool changed READ hasChanged)

	public:
		explicit KeyStoreQt(QObject *parent = 0);

	private:
		typedef QObject super;
		Q_DISABLE_COPY(KeyStoreQt)

	protected:
		/** Pure virtual functions for base class signals. **/

		/**
		 * A key has changed.
		 * @param sectIdx Section index.
		 * @param keyIdx Key index.
		 */
		void keyChanged_int(int sectIdx, int keyIdx) final;

		/**
		 * A key has changed.
		 * @param idx Flat key index.
		 */
		void keyChanged_int(int idx) final;

		/**
		 * All keys have changed.
		 */
		void allKeysChanged_int(void) final;

		/**
		 * KeyStore has been changed by the user.
		 */
		void modified_int(void) final;

	signals:
		/** Qt signals. **/

		/**
		 * A key has changed.
		 * @param sectIdx Section index.
		 * @param keyIdx Key index.
		 */
		void keyChanged(int sectIdx, int keyIdx);

		/**
		 * A key has changed.
		 * @param idx Flat key index.
		 */
		void keyChanged(int idx);

		/**
		 * All keys have changed.
		 */
		void allKeysChanged(void);

		/**
		 * KeyStore has been changed by the user.
		 */
		void modified(void);
};

#endif /* __ROMPROPERTIES_KDE_CONFIG_KEYSTOREQT_HPP__ */
