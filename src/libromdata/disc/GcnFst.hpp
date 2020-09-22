/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * GcnFst.hpp: GameCube/Wii FST parser.                                    *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_DISC_GCNFST_HPP__
#define __ROMPROPERTIES_LIBROMDATA_DISC_GCNFST_HPP__

#include "librpbase/disc/IFst.hpp"

namespace LibRomData {

class GcnFstPrivate;
class GcnFst : public LibRpBase::IFst
{
	public:
		/**
		 * Parse a GameCube FST.
		 * @param fstData FST data.
		 * @param len Length of fstData, in bytes.
		 * @param offsetShift File offset shift. (0 = GCN, 2 = Wii)
		 */
		ATTR_ACCESS_SIZE(read_only, 2, 3)
		GcnFst(const uint8_t *fstData, uint32_t len, uint8_t offsetShift);
		virtual ~GcnFst();

	private:
		typedef IFst super;
		RP_DISABLE_COPY(GcnFst)

	private:
		friend class GcnFstPrivate;
		GcnFstPrivate *const d;

	public:
		/**
		 * Is the FST open?
		 * @return True if open; false if not.
		 */
		bool isOpen(void) const final;

		/**
		 * Have any errors been detected in the FST?
		 * @return True if yes; false if no.
		 */
		bool hasErrors(void) const final;

	public:
		/** opendir() interface. **/

		/**
		 * Open a directory.
		 * @param path	[in] Directory path.
		 * @return Dir*, or nullptr on error.
		 */
		Dir *opendir(const char *path) final;

		/**
		 * Read a directory entry.
		 * @param dirp Dir pointer.
		 * @return DirEnt*, or nullptr if end of directory or on error.
		 * (TODO: Add lastError()?)
		 */
		DirEnt *readdir(Dir *dirp) final;

		/**
		 * Close an opened directory.
		 * @param dirp Dir pointer.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		int closedir(Dir *dirp) final;

		/**
		 * Get the directory entry for the specified file.
		 * @param filename	[in] Filename.
		 * @param dirent	[out] Pointer to DirEnt buffer.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		int find_file(const char *filename, DirEnt *dirent) final;

	public:
		/**
		 * Get the total size of all files.
		 *
		 * This is a shortcut function that reads the FST
		 * directly instead of using opendir().
		 *
		 * @return Size of all files, in bytes. (-1 on error)
		 */
		off64_t totalUsedSize(void) const;
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_DISC_GCNFST_HPP__ */
