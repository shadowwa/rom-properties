/***************************************************************************
 * ROM Properties Page shell extension. (libcachemgr)                      *
 * UrlmonDownloader.cpp: urlmon-based file downloader.                     *
 *                                                                         *
 * Copyright (c) 2016 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "stdafx.h"
#include "UrlmonDownloader.hpp"

#include "libromdata/TextFuncs.hpp"
#include "libromdata/RpFile.hpp"
#include "libromdata/RpWin32.hpp"
using LibRomData::IRpFile;
using LibRomData::RpFile;
using LibRomData::rp_string;

// C includes. (C++ namespace)
#include <cassert>
#include <cstring>

// C++ includes.
#include <string>
using std::string;

// Windows includes.
#include <urlmon.h>

namespace LibCacheMgr {

UrlmonDownloader::UrlmonDownloader()
	: super()
{ }

UrlmonDownloader::UrlmonDownloader(const rp_char *url)
	: super(url)
{ }

UrlmonDownloader::UrlmonDownloader(const rp_string &url)
	: super(url)
{ }

/**
 * Download the file.
 * @return 0 on success; non-zero on error. [TODO: HTTP error codes?]
 */
int UrlmonDownloader::download(void)
{
	// Reference: https://msdn.microsoft.com/en-us/library/ms775122(v=vs.85).aspx
	// TODO: IBindStatusCallback to enforce data size?
	// TODO: Check Content-Length to prevent large files in the first place?
	// TODO: Replace with WinInet?

	// Buffer for cache filename.
	wchar_t szFileName[MAX_PATH];

	HRESULT hr = URLDownloadToCacheFile(nullptr, RP2W_s(m_url),
		szFileName, sizeof(szFileName)/sizeof(szFileName[0]),
		0, nullptr /* TODO */);

	if (FAILED(hr)) {
		// Failed to download the file.
		return hr;
	}

	// Open the cached file.
	IRpFile *file = new RpFile(W2RP_c(szFileName), RpFile::FM_OPEN_READ);
	if (!file || !file->isOpen()) {
		// Unable to open the file.
		delete file;
		return -1;
	}
	// TODO: Remove this after Gdiplus testing.
	m_cacheFile = szFileName;

	// Read the file into the data buffer.
	// TODO: malloc()'d buffer to prevent initialization?
	int64_t fileSize = file->fileSize();
	m_data.resize((size_t)fileSize);
	size_t ret = file->read(m_data.data(), (size_t)fileSize);
	if (ret != fileSize) {
		// Error reading the file.
		m_data.clear();
		return -2;
	}

	// Data loaded.
	// TODO: Delete the cached file?
	return 0;
}

}