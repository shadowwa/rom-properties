/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * FileFormat.cpp: Texture file format base class.                         *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "FileFormat.hpp"
#include "FileFormat_p.hpp"

// librpfile
using LibRpFile::IRpFile;

// librpthreads
#include "librpthreads/Atomics.h"

namespace LibRpTexture {

/** FileFormatPrivate **/

/**
 * Initialize a FileFormatPrivate storage class.
 *
 * @param q FileFormat class.
 * @param file Texture file.
 */
FileFormatPrivate::FileFormatPrivate(FileFormat *q, IRpFile *file)
	: q_ptr(q)
	, ref_cnt(1)
	, isValid(false)
	, file(nullptr)
	, mimeType(nullptr)
{
	// Clear the arrays.
	memset(dimensions, 0, sizeof(dimensions));

	if (file) {
		// Reference the file.
		this->file = file->ref();
	}
}

FileFormatPrivate::~FileFormatPrivate()
{
	// Unreference the file.
	UNREF(this->file);
}

/** FileFormat **/

FileFormat::FileFormat(FileFormatPrivate *d)
	: d_ptr(d)
{ }

FileFormat::~FileFormat()
{
	delete d_ptr;
}

/**
 * Is this ROM valid?
 * @return True if it is; false if it isn't.
 */
bool FileFormat::isValid(void) const
{
	RP_D(const FileFormat);
	return d->isValid;
}

/**
 * Is the file open?
 * @return True if the file is open; false if it isn't.
 */
bool FileFormat::isOpen(void) const
{
	RP_D(const FileFormat);
	return (d->file != nullptr);
}

/**
 * Close the opened file.
 */
void FileFormat::close(void)
{
	// Unreference the file.
	RP_D(FileFormat);
	UNREF_AND_NULL(d->file);
}

/** Property accessors **/

/**
 * Get the file's MIME type.
 * @return MIME type, or nullptr if unknown.
 */
const char *FileFormat::mimeType(void) const
{
	RP_D(const FileFormat);
	return d->mimeType;
}

/**
 * Get the image width.
 * @return Image width.
 */
int FileFormat::width(void) const
{
	RP_D(const FileFormat);
	return d->dimensions[0];
}

/**
 * Get the image height.
 * @return Image height.
 */
int FileFormat::height(void) const
{
	RP_D(const FileFormat);
	return d->dimensions[1];
}

/**
 * Get the image dimensions.
 * If the image is 2D, then 'z' will be set to zero.
 * @param pBuf Three-element array for [x, y, z].
 * @return 0 on success; negative POSIX error code on error.
 */
int FileFormat::getDimensions(int pBuf[3]) const
{
	RP_D(const FileFormat);
	if (!d->isValid) {
		// Not supported.
		return -EBADF;
	}

	memcpy(pBuf, d->dimensions, sizeof(d->dimensions));
	return 0;
}

}
