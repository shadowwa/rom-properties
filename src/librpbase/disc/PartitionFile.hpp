/***************************************************************************
 * ROM Properties Page shell extension. (librpbase)                        *
 * PartitionFile.hpp: IRpFile implementation for IPartition.               *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPBASE_DISC_PARTITIONFILE_HPP__
#define __ROMPROPERTIES_LIBRPBASE_DISC_PARTITIONFILE_HPP__

#include "librpfile/IRpFile.hpp"

namespace LibRpBase {

class IDiscReader;

class PartitionFile : public LibRpFile::IRpFile
{
	public:
		/**
		 * Open a file from an IPartition.
		 * NOTE: These files are read-only.
		 * @param partition	[in] IPartition (or IDiscReader) object.
		 * @param offset	[in] File starting offset.
		 * @param size		[in] File size.
		 */
		PartitionFile(IDiscReader *partition, off64_t offset, off64_t size);
	protected:
		virtual ~PartitionFile();	// call unref() instead

	private:
		typedef IRpFile super;
		RP_DISABLE_COPY(PartitionFile)

	public:
		/**
		 * Is the file open?
		 * This usually only returns false if an error occurred.
		 * @return True if the file is open; false if it isn't.
		 */
		bool isOpen(void) const final;

		/**
		 * Close the file.
		 */
		void close(void) final;

		/**
		 * Read data from the file.
		 * @param ptr Output data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes read.
		 */
		ATTR_ACCESS_SIZE(write_only, 2, 3)
		size_t read(void *ptr, size_t size) final;

		/**
		 * Write data to the file.
		 * (NOTE: Not valid for PartitionFile; this will always return 0.)
		 * @param ptr Input data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes written.
		 */
		ATTR_ACCESS_SIZE(read_only, 2, 3)
		size_t write(const void *ptr, size_t size) final;

		/**
		 * Set the file position.
		 * @param pos File position.
		 * @return 0 on success; -1 on error.
		 */
		int seek(off64_t pos) final;

		/**
		 * Get the file position.
		 * @return File position, or -1 on error.
		 */
		off64_t tell(void) final;

	public:
		/** File properties. **/

		/**
		 * Get the file size.
		 * @return File size, or negative on error.
		 */
		off64_t size(void) final;

		/**
		 * Get the filename.
		 * @return Filename. (May be empty if the filename is not available.)
		 */
		std::string filename(void) const final;

	protected:
		IDiscReader *m_partition;
		off64_t m_offset;	// File starting offset.
		off64_t m_size;		// File size.
		off64_t m_pos;		// Current position.
};

}

#endif /* __ROMPROPERTIES_LIBRPBASE_DISC_PARTITIONFILE_HPP__ */
