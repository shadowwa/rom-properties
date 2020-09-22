/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * IsoPartition.hpp: ISO-9660 partition reader.                            *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_DISC_ISOPARTITION_HPP__
#define __ROMPROPERTIES_LIBROMDATA_DISC_ISOPARTITION_HPP__

#include "librpbase/disc/IPartition.hpp"
//#include "librpbase/disc/IFst.hpp"

namespace LibRomData {

class IsoPartitionPrivate;
class IsoPartition : public LibRpBase::IPartition
{
	public:
		/**
		 * Construct an IsoPartition with the specified IDiscReader.
		 *
		 * NOTE: The IDiscReader *must* remain valid while this
		 * IsoPartition is open.
		 *
		 * @param discReader IDiscReader.
		 * @param partition_offset Partition start offset.
		 * @param iso_start_offset ISO start offset, in blocks. (If -1, uses heuristics.)
		 */
		IsoPartition(IDiscReader *discReader, off64_t partition_offset, int iso_start_offset = -1);
	protected:
		virtual ~IsoPartition();	// call unref() instead

	private:
		typedef IPartition super;
		RP_DISABLE_COPY(IsoPartition)

	protected:
		friend class IsoPartitionPrivate;
		IsoPartitionPrivate *const d_ptr;

	public:
		/** IDiscReader **/

		/**
		 * Read data from the partition.
		 * @param ptr Output data buffer.
		 * @param size Amount of data to read, in bytes.
		 * @return Number of bytes read.
		 */
		ATTR_ACCESS_SIZE(write_only, 2, 3)
		size_t read(void *ptr, size_t size) override;

		/**
		 * Set the partition position.
		 * @param pos Partition position.
		 * @return 0 on success; -1 on error.
		 */
		int seek(off64_t pos) override;

		/**
		 * Get the partition position.
		 * @return Partition position on success; -1 on error.
		 */
		off64_t tell(void) override;

		/**
		 * Get the data size.
		 * This size does not include the partition header,
		 * and it's adjusted to exclude hashes.
		 * @return Data size, or -1 on error.
		 */
		off64_t size(void) final;

	public:
		/** IPartition **/

		/**
		 * Get the partition size.
		 * This size includes the partition header and hashes.
		 * @return Partition size, or -1 on error.
		 */
		off64_t partition_size(void) const final;

		/**
		 * Get the used partition size.
		 * This size includes the partition header and hashes,
		 * but does not include "empty" sectors.
		 * @return Used partition size, or -1 on error.
		 */
		off64_t partition_size_used(void) const final;

	public:
		/** IFst wrapper functions. **/

		// TODO
#if 0
		/**
		 * Open a directory.
		 * @param path	[in] Directory path.
		 * @return IFst::Dir*, or nullptr on error.
		 */
		LibRpBase::IFst::Dir *opendir(const char *path);

		/**
		 * Open a directory.
		 * @param path	[in] Directory path.
		 * @return IFst::Dir*, or nullptr on error.
		 */
		inline LibRpBase::IFst::Dir *opendir(const std::string &path)
		{
			return opendir(path.c_str());
		}

		/**
		 * Read a directory entry.
		 * @param dirp IFst::Dir pointer.
		 * @return IFst::DirEnt, or nullptr if end of directory or on error.
		 * (TODO: Add lastError()?)
		 */
		LibRpBase::IFst::DirEnt *readdir(LibRpBase::IFst::Dir *dirp);

		/**
		 * Close an opened directory.
		 * @param dirp IFst::Dir pointer.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		int closedir(LibRpBase::IFst::Dir *dirp);
#endif

		/**
		 * Open a file. (read-only)
		 * @param filename Filename.
		 * @return IRpFile*, or nullptr on error.
		 */
		LibRpFile::IRpFile *open(const char *filename);

		/**
		 * Get a file's timestamp.
		 * @param filename Filename.
		 * @return Timestamp, or -1 on error.
		 */
		time_t get_mtime(const char *filename);
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_DISC_ISOPARTITION_HPP__ */
