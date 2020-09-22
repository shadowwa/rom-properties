/***************************************************************************
 * ROM Properties Page shell extension. (libcachecommon)                   *
 * CacheDir.cpp: Cache directory handler.                                  *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "CacheDir.hpp"

// C includes. (C++ namespace)
#include <cassert>
#include <cerrno>

// C++ STL classes.
using std::string;

// librpthreads
#include "librpthreads/pthread_once.h"

// OS-specific userdirs
#ifdef _WIN32
# include "libwin32common/userdirs.hpp"
# define OS_NAMESPACE LibWin32Common
# define DIR_SEP_CHR '\\'
#else
# include "libunixcommon/userdirs.hpp"
# define OS_NAMESPACE LibUnixCommon
# define DIR_SEP_CHR '/'
#endif

namespace LibCacheCommon {

/** Configuration directories. **/
// pthread_once() control variable.
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
// User's cache directory.
static string cache_dir;

/**
 * Initialize the cache directory.
 * Called by pthread_once().
 */
static void initCacheDirectory(void)
{
	// Uses LibUnixCommon or LibWin32Common, depending on platform.
	cache_dir = OS_NAMESPACE::getCacheDirectory();
	if (cache_dir.empty())
		return;

	// Add a trailing slash if necessary.
	if (cache_dir.at(cache_dir.size()-1) != DIR_SEP_CHR)
		cache_dir += DIR_SEP_CHR;
#ifdef _WIN32
	// Append "rom-properties\\cache".
	cache_dir += "rom-properties\\cache";
#else /* !_WIN32 */
	// Append "rom-properties".
	cache_dir += "rom-properties";
#endif /* _WIN32 */
}

/**
 * Get the cache directory.
 *
 * NOTE: May return an empty string if the cache directory
 * isn't accessible, e.g. when running under bubblewrap.
 *
 * @return Cache directory, or empty string on error.
 */
const std::string &getCacheDirectory(void)
{
	pthread_once(&once_control, initCacheDirectory);
	return cache_dir;
}

}
