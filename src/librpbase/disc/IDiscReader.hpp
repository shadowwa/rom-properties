/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * IDiscReader.hpp: Disc reader interface.                                 *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_IDISCREADER_HPP__
#define __ROMPROPERTIES_LIBRPBASE_IDISCREADER_HPP__

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cstddef>

// common macros
#include "common.h"
#include "RefBase.hpp"

namespace LibRpFile {
	class IRpFile;
}

namespace LibRpBase {

class IDiscReader : public RefBase
{
	protected:
		explicit IDiscReader(LibRpFile::IRpFile *file);
		explicit IDiscReader(IDiscReader *discReader);
	protected:
		virtual ~IDiscReader();	// call unref() instead

	private:
		RP_DISABLE_COPY(IDiscReader)

	public:
		inline IDiscReader *ref(void)
		{
			return RefBase::ref<IDiscReader>();
		}

	public:
		/** Disc image detection functions. **/

		// TODO: Move RomData::DetectInfo somewhere else and use it here?
		/**
		 * Is a disc image supported by this object?
		 * @param pHeader Disc image header.
		 * @param szHeader Size of header.
		 * @return Class-specific disc format ID (>= 0) if supported; -1 if not.
		 */
		virtual int isDiscSupported(const uint8_t *pHeader, size_t szHeader) const = 0;

	public:
		/**
		 * Is the disc image open?
		 * This usually only returns false if an error occurred.
		 * @return True if the disc image is open; false if it isn't.
		 */
		bool isOpen(void) const;

		/**
		 * Get the last error.
		 * @return Last POSIX error, or 0 if no error.
		 */
		inline int lastError(void) const
		{
			return m_lastError;
		}

		/**
		 * Clear the last error.
		 */
		inline void clearError(void)
		{
			m_lastError = 0;
		}

		/**
		 * Read data from the disc image.
		 * @param ptr Output data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes read.
		 */
		ATTR_ACCESS_SIZE(write_only, 2, 3)
		virtual size_t read(void *ptr, size_t size) = 0;

		/**
		 * Set the disc image position.
		 * @param pos disc image position.
		 * @return 0 on success; -1 on error.
		 */
		virtual int seek(off64_t pos) = 0;

		/**
		 * Seek to the beginning of the file.
		 */
		inline void rewind(void)
		{
			this->seek(0);
		}

		/**
		 * Get the disc image position.
		 * @return Disc image position on success; -1 on error.
		 */
		virtual off64_t tell(void) = 0;

		/**
		 * Get the disc image size.
		 * @return Disc image size, or -1 on error.
		 */
		virtual off64_t size(void) = 0;

	public:
		/** Convenience functions implemented for all IRpFile classes. **/

		/**
		 * Seek to the specified address, then read data.
		 * @param pos	[in] Requested seek address.
		 * @param ptr	[out] Output data buffer.
		 * @param size	[in] Amount of data to read, in bytes.
		 * @return Number of bytes read on success; 0 on seek or read error.
		 */
		ATTR_ACCESS_SIZE(write_only, 3, 4)
		size_t seekAndRead(off64_t pos, void *ptr, size_t size);

	public:
		/** Device file functions **/

		/**
		 * Is the underlying file a device file?
		 * @return True if the underlying file is a device file; false if not.
		 */
		bool isDevice(void) const;

	protected:
		// Subclasses may have an underlying file, or may
		// stack another IDiscReader object.
		union {
			LibRpFile::IRpFile *m_file;
			IDiscReader *m_discReader;
		};
		bool m_hasDiscReader;

		int m_lastError;
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_IDISCREADER_HPP__ */
