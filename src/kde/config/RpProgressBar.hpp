/***************************************************************************
 * ROM Properties Page shell extension. (KDE4/KF5)                         *
 * RpProgressBar.hpp: QProgressBar subclass with error status support.     *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_KDE_CONFIG_RPPROGRESSBAR_HPP__
#define __ROMPROPERTIES_KDE_CONFIG_RPPROGRESSBAR_HPP__

// Qt includes
#include <QProgressBar>
#include <QStyle>

class RpProgressBar : public QProgressBar
{
	Q_OBJECT

	Q_PROPERTY(bool error READ hasError WRITE setError NOTIFY errorChanged)

	public:
		explicit RpProgressBar(QWidget *parent = 0)
			: super(parent)
			, m_error(false)
		{ }

	private:
		typedef QProgressBar super;
		Q_DISABLE_COPY(RpProgressBar)

	public:
		/**
		 * Set the error state.
		 * @param error True if error; false if not.
		 */
		void setError(bool error);

		/**
		 * Get the error state.
		 * @return Error state.
		 */
		bool hasError(void) const
		{
			return m_error;
		}

	signals:
		/**
		 * Error state has changed.
		 * @param error New error state.
		 */
		void errorChanged(bool error);

	private:
		bool m_error;
};

#endif /* __ROMPROPERTIES_KDE_CONFIG_RPPROGRESSBAR_HPP__ */
