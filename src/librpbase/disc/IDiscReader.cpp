/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * IDiscReader.cpp: Disc reader interface.                                 *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "IDiscReader.hpp"

// librpfile
using LibRpFile::IRpFile;

namespace LibRpBase {

IDiscReader::IDiscReader(IRpFile *file)
	: m_hasDiscReader(false)
	, m_lastError(0)
{
	if (file) {
		m_file = file->ref();
	} else {
		m_file = nullptr;
	}
}

IDiscReader::IDiscReader(IDiscReader *discReader)
	: m_hasDiscReader(true)
	, m_lastError(0)
{
	if (discReader) {
		m_discReader = discReader->ref();
	} else {
		m_discReader = nullptr;
	}
}

IDiscReader::~IDiscReader()
{
	if (!m_hasDiscReader) {
		UNREF(m_file);
	} else {
		UNREF(m_discReader);
	}
}

/**
 * Is the disc image open?
 * This usually only returns false if an error occurred.
 * @return True if the disc image is open; false if it isn't.
 */
bool IDiscReader::isOpen(void) const
{
	if (!m_hasDiscReader) {
		return (m_file && m_file->isOpen());
	} else {
		return (m_discReader && m_discReader->isOpen());
	}
}

/**
 * Seek to the specified address, then read data.
 * @param pos	[in] Requested seek address.
 * @param ptr	[out] Output data buffer.
 * @param size	[in] Amount of data to read, in bytes.
 * @return Number of bytes read on success; 0 on seek or read error.
 */
size_t IDiscReader::seekAndRead(off64_t pos, void *ptr, size_t size)
{
	int ret = this->seek(pos);
	if (ret != 0) {
		// Seek error.
		return 0;
	}
	return this->read(ptr, size);
}

/** Device file functions **/

/**
 * Is the underlying file a device file?
 * @return True if the underlying file is a device file; false if not.
 */
bool IDiscReader::isDevice(void) const
{
	if (!m_hasDiscReader) {
		return (m_file && m_file->isDevice());
	} else {
		return (m_discReader && m_discReader->isDevice());
	}
}

}
