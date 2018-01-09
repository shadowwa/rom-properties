/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * KeyStoreQt.hpp: Key store object for Qt.                                *
 *                                                                         *
 * Copyright (c) 2012-2017 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
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
		virtual ~KeyStoreQt();

	private:
		typedef QObject super;
		Q_DISABLE_COPY(KeyStoreQt)

	protected:
		/** Pure virtual functions for Qt signals. **/

		/**
		 * A key has changed.
		 * @param sectIdx Section index.
		 * @param keyIdx Key index.
		 */
		virtual void keyChanged_int(int sectIdx, int keyIdx) override final;

		/**
		 * A key has changed.
		 * @param idx Flat key index.
		 */
		virtual void keyChanged_int(int idx) override final;

		/**
		 * All keys have changed.
		 */
		virtual void allKeysChanged_int(void) override final;

		/**
		 * KeyStore has been changed by the user.
		 */
		virtual void modified_int(void) override final;

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
