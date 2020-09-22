/***************************************************************************
 * ROM Properties Page shell extension. (KDE)                              *
 * KeyStoreModel.hpp: QAbstractListModel for KeyStore.                     *
 *                                                                         *
 * Copyright (c) 2012-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_CONFIG_KEYSTOREMODEL_HPP__
#define __ROMPROPERTIES_KDE_CONFIG_KEYSTOREMODEL_HPP__

// Qt includes.
#include <QtCore/QAbstractItemModel>

class KeyStoreQt;

class KeyStoreModelPrivate;
class KeyStoreModel : public QAbstractItemModel
{
	Q_OBJECT
	Q_PROPERTY(KeyStoreQt* keyStore READ keyStore WRITE setKeyStore NOTIFY keyStoreChanged)

	public:
		explicit KeyStoreModel(QObject *parent = 0);
		virtual ~KeyStoreModel();

	private:
		typedef QAbstractItemModel super;
		KeyStoreModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(KeyStoreModel)
		Q_DISABLE_COPY(KeyStoreModel)

	public:
		enum class Column {
			KeyName	= 0,
			Value	= 1,
			IsValid	= 2,

			Max
		};

		// Qt Model/View interface.
		int rowCount(const QModelIndex& parent = QModelIndex()) const final;
		int columnCount(const QModelIndex& parent = QModelIndex()) const final;

		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const final;
		QModelIndex parent(const QModelIndex& index) const final;

		// Custom role for "allowKanji".
		static const int AllowKanjiRole = Qt::UserRole;

		QVariant data(const QModelIndex& index, int role) const final;
		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final;
		Qt::ItemFlags flags(const QModelIndex &index) const final;

		QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

	public:
		/**
		 * Set the KeyStore to use in this model.
		 * @param keyStore KeyStore.
		 */
		void setKeyStore(KeyStoreQt *keyStore);

		/**
		 * Get the KeyStore in use by this model.
		 * @return KeyStore.
		 */
		KeyStoreQt *keyStore(void) const;

	signals:
		/**
		 * The KeyStore has been changed.
		 */
		void keyStoreChanged(void);

	private slots:
		/**
		 * KeyStore object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void keyStore_destroyed_slot(QObject *obj = 0);

		/**
		 * A key in the KeyStore has changed.
		 * @param sectIdx Section index.
		 * @param keyIdx Key index.
		 */
		void keyStore_keyChanged_slot(int sectIdx, int keyIdx);

		/**
		 * All keys in the KeyStore have changed.
		 */
		void keyStore_allKeysChanged_slot(void);

		/**
		 * The system theme has changed.
		 */
		void themeChanged_slot(void);
};

#endif /* __ROMPROPERTIES_KDE_CONFIG_KEYSTOREMODEL_HPP__ */
