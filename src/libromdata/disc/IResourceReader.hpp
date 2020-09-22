/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * IResourceReader.hpp: Interface for Windows resource readers.            *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_DISC_IRESOURCEREADER_HPP__
#define __ROMPROPERTIES_LIBROMDATA_DISC_IRESOURCEREADER_HPP__

#include "../Other/exe_structs.h"

// librpbase
#include "librpbase/config.librpbase.h"
#include "librpbase/disc/IPartition.hpp"

// C++ includes.
#include <string>
#include <unordered_map>
#include <vector>

namespace LibRomData {

class IResourceReader : public LibRpBase::IPartition
{
	protected:
		IResourceReader(LibRpFile::IRpFile *file) : super(file) { }
	protected:
		virtual ~IResourceReader() = 0;	// call unref() instead

	private:
		typedef LibRpBase::IPartition super;
		RP_DISABLE_COPY(IResourceReader)

	public:
		/**
		 * DWORD alignment function.
		 * @param file	[in] File to DWORD align.
		 * @return 0 on success; non-zero on error.
		 */
		static int alignFileDWORD(LibRpFile::IRpFile *file);

	public:
		/** Resource access functions. **/

		/**
		 * Open a resource.
		 * @param type Resource type ID.
		 * @param id Resource ID. (-1 for "first entry")
		 * @param lang Language ID. (-1 for "first entry")
		 * @return IRpFile*, or nullptr on error.
		 */
		virtual LibRpFile::IRpFile *open(uint16_t type, int id, int lang) = 0;

		// StringTable.
		// - Element 1: Key
		// - Element 2: Value
		typedef std::vector<std::pair<std::string, std::string> > StringTable;

		// StringFileInfo section.
		// - Key: Langauge ID. (LOWORD = charset, HIWORD = language)
		// - Value: String table.
		typedef std::unordered_map<uint32_t, StringTable> StringFileInfo;

		/**
		 * Load a VS_VERSION_INFO resource.
		 * Data will be byteswapped to host-endian if necessary.
		 *
		 * @param id		[in] Resource ID. (-1 for "first entry")
		 * @param lang		[in] Language ID. (-1 for "first entry")
		 * @param pVsFfi	[out] VS_FIXEDFILEINFO (host-endian)
		 * @param pVsSfi	[out] StringFileInfo section.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int load_VS_VERSION_INFO(int id, int lang, VS_FIXEDFILEINFO *pVsFfi, StringFileInfo *pVsSfi) = 0;
};

/**
 * Both gcc and MSVC fail to compile unless we provide
 * an empty implementation, even though the function is
 * declared as pure-virtual.
 */
inline IResourceReader::~IResourceReader() { }

}

#endif /* __ROMPROPERTIES_LIBROMDATA_DISC_IRESOURCEREADER_HPP__ */
