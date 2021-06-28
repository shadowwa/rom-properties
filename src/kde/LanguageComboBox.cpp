/***************************************************************************
 * ROM Properties Page shell extension. (KDE4/KF5)                         *
 * LanguageComboBox.cpp: Language QComboBox subclass.                      *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "LanguageComboBox.hpp"

// librpbase
using LibRpBase::SystemRegion;

// C++ STL classes
using std::set;

LanguageComboBox::LanguageComboBox(QWidget *parent)
	: super(parent)
	, m_forcePAL(false)
{
	connect(this, SIGNAL(currentIndexChanged(int)),
		this, SLOT(this_currentIndexChanged_slot(int)));
}

/**
 * Update all icons.
 */
void LanguageComboBox::updateIcons(void)
{
	const int count = this->count();
	if (count <= 0)
		return;

	// Sprite sheets. (32x32, 24x24, 16x16)
	const QPixmap spriteSheets[3] = {
		QPixmap(QLatin1String(":/flags/flags-32x32.png")),
		QPixmap(QLatin1String(":/flags/flags-24x24.png")),
		QPixmap(QLatin1String(":/flags/flags-16x16.png")),
	};

	for (int i = 0; i < count; i++) {
		int col, row;
		const uint32_t lc = this->itemData(i).toUInt();
		if (SystemRegion::getFlagPosition(lc, &col, &row, m_forcePAL) != 0) {
			// No icon available.
			this->setItemIcon(i, QIcon());
		}

		// Found a matching icon.
		QIcon flag_icon;
		flag_icon.addPixmap(spriteSheets[0].copy(col*32, row*32, 32, 32));
		flag_icon.addPixmap(spriteSheets[1].copy(col*24, row*24, 24, 24));
		flag_icon.addPixmap(spriteSheets[2].copy(col*16, row*16, 16, 16));
		this->setItemIcon(i, flag_icon);
	}
}

/**
 * Set the language codes.
 * @param set_lc Set of language codes.
 */
void LanguageComboBox::setLCs(const std::set<uint32_t> &set_lc)
{
	// Check the LC of the selected index.
	const uint32_t sel_lc = selectedLC();

	// Clear the QComboBox and repopulate it.
	this->clear();

	int sel_idx = -1;
	int cur_idx = 0;
	const auto set_lc_cend = set_lc.cend();
	for (auto iter = set_lc.cbegin(); iter != set_lc_cend; ++iter, cur_idx++) {
		const uint32_t lc = *iter;
		const char *const name = SystemRegion::getLocalizedLanguageName(lc);
		if (name) {
			this->addItem(U82Q(name), lc);
		} else {
			// Invalid language code.
			this->addItem(lcToQString(lc), lc);
		}

		if (sel_lc != 0 && lc == sel_lc) {
			// This was the previously-selected LC.
			sel_idx = cur_idx;
		}
	}

	// Update the icons.
	updateIcons();

	// Re-select the previously-selected LC.
	this->setCurrentIndex(sel_idx);

	// Language codes updated.
	// TODO: Emit a signal?
}

/**
 * Set the language codes.
 * @param p_lc Array of language codes.
 * @param len Number of language codes.
 */
ATTR_ACCESS_SIZE(read_only, 2, 3)
void LanguageComboBox::setLCs(const uint32_t *p_lc, size_t len)
{
	// Convert the array to std::set<uint32_t> first.
	std::set<uint32_t> set_lc;
	for (; len > 0; len--, p_lc++) {
		set_lc.emplace(*p_lc);
	}
	setLCs(set_lc);
}

/**
 * Get the set of language codes.
 * @return Set of language codes.
 */
std::set<uint32_t> LanguageComboBox::lcs(void) const
{
	std::set<uint32_t> set_lc;
	const int count = this->count();
	for (int i = 0; i < count; i++) {
		set_lc.insert(this->itemData(i).toUInt());
	}
	return set_lc;
}

/**
 * Clear the language codes.
 */
void LanguageComboBox::clearLCs()
{
	const int cur_idx = this->currentIndex();
	this->clear();
	if (cur_idx >= 0) {
		// Nothing is selected now.
		emit lcChanged(0);
	}
}

/**
 * Set the selected language code.
 *
 * NOTE: This function will return true if the LC was found,
 * even if it was already selected.
 *
 * @param lc Language code. (0 to unselect)
 * @return True if set; false if LC was not found.
 */
bool LanguageComboBox::setSelectedLC(uint32_t lc)
{
	if (lc == 0) {
		// Unselect the selected LC.
		if (this->currentIndex() != -1) {
			this->setCurrentIndex(-1);
			emit lcChanged(0);
		}
		return true;
	}

	int index = this->findData(lc);
	if (index >= 0) {
		if (index != this->currentIndex()) {
			this->setCurrentIndex(index);
			emit lcChanged(lc);
		}
		return true;
	}

	// Language code not found.
	return false;
}

/**
 * Get the selected language code.
 * @return Selected language code. (0 if none)
 */
uint32_t LanguageComboBox::selectedLC(void) const
{
	const int index = this->currentIndex();
	return (index >= 0 ? this->itemData(index).toUInt() : 0);
}

/**
 * Set the Force PAL setting.
 * @param forcePAL Force PAL setting.
 */
void LanguageComboBox::setForcePAL(bool forcePAL)
{
	if (m_forcePAL == forcePAL)
		return;
	m_forcePAL = forcePAL;
	updateIcons();
}

/**
 * Emit lcChanged() on currentIndexChanged().
 * @param index Current index.
 */
void LanguageComboBox::this_currentIndexChanged_slot(int index)
{
	if (index < 0) {
		emit lcChanged(0);
		return;
	}

	emit lcChanged(this->itemData(index).toUInt());
}
