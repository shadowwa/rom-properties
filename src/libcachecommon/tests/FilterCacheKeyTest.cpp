/***************************************************************************
 * ROM Properties Page shell extension. (libcachecommon/tests)             *
 * FilterCacheKeyTest.cpp: CacheManager::filterCacheKey() test.            *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

// Google Test
#include "gtest/gtest.h"
#include "tcharx.h"

// libromdata
#include "librpbase/TextFuncs.hpp"

// libcachecommon
#include "../CacheKeys.hpp"

// C++ includes.
#include <string>
using std::string;

namespace LibCacheCommon { namespace Tests {

struct FilterCacheKeyTest_mode
{
	// Cache keys.
	const char *keyOrig;			// Original key.
	const char *keyFilteredPosix;	// Filtered key. (POSIX)
	const char *keyFilteredWin32;	// Filtered key. (Win32)

	FilterCacheKeyTest_mode(
		const char *keyOrig,
		const char *keyFilteredPosix,
		const char *keyFilteredWin32)
		: keyOrig(keyOrig)
		, keyFilteredPosix(keyFilteredPosix)
		, keyFilteredWin32(keyFilteredWin32)
	{ }

	// May be required for MSVC 2010?
	FilterCacheKeyTest_mode(const FilterCacheKeyTest_mode &other)
		: keyOrig(other.keyOrig)
		, keyFilteredPosix(other.keyFilteredPosix)
		, keyFilteredWin32(other.keyFilteredWin32)
	{ }

	// Required for MSVC 2010.
	FilterCacheKeyTest_mode &operator=(const FilterCacheKeyTest_mode &other)
	{
		keyOrig = other.keyOrig;
		keyFilteredPosix = other.keyFilteredPosix;
		keyFilteredWin32 = other.keyFilteredWin32;
		return *this;
	}
};

class FilterCacheKeyTest : public ::testing::TestWithParam<FilterCacheKeyTest_mode>
{
	protected:
		FilterCacheKeyTest() { }
};

/**
 * Run a CacheManager::filterCacheKey() test.
 */
TEST_P(FilterCacheKeyTest, filterCacheKey)
{
	const FilterCacheKeyTest_mode &mode = GetParam();

	string keyFiltered = mode.keyOrig;
	int ret = LibCacheCommon::filterCacheKey(keyFiltered);

	// If it starts with certain invalid characters, we should expect -EINVAL.
	// Otherwise, we'll expect 0.
	if (mode.keyOrig[0] == '/' ||
	    mode.keyOrig[0] == '\\' ||
	    mode.keyOrig[0] == '.' ||
	    mode.keyOrig[1] == ':')
	{
		EXPECT_EQ(-EINVAL, ret);
		return;
	}

	// Expecting success.
	EXPECT_EQ(0, ret);
	if (ret != 0) {
		return;
	}

#ifdef _WIN32
	EXPECT_EQ(mode.keyFilteredWin32, keyFiltered);
#else /* !_WIN32 */
	EXPECT_EQ(mode.keyFilteredPosix, keyFiltered);
#endif /* _WIN32 */
}

// TODO: Add more test cases.
INSTANTIATE_TEST_SUITE_P(CacheManagerTest, FilterCacheKeyTest,
	::testing::Values(
		// Known-good cache key.
		FilterCacheKeyTest_mode(
			"wii/disc/US/GALE01.png",
			"wii/disc/US/GALE01.png",
			"wii\\disc\\US\\GALE01.png"),

		// Simple ".." traversal.
		FilterCacheKeyTest_mode(
			"../../../../etc/passwd",
			"",
			""),

		// "..." traversal, which isn't actually traversal,
		// but is filtered out anyway.
		FilterCacheKeyTest_mode(
			".../.../.../.../etc/passwd",
			"",
			""),

		// Unix-style absolute path. (blocked due to leading '/')
		FilterCacheKeyTest_mode(
			"/etc/passwd",
			"",
			""),

		// Windows-style absolute path. (blocked due to ':')
		FilterCacheKeyTest_mode(
			"C:/Windows/System32/config/SAM",
			"",
			""),

		// Filter out bad characters.
		// These characters are converted to '_', unlike '\\' and ':',
		// which abort processing and return an empty string.
		FilterCacheKeyTest_mode(
			"lol/\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\x20\"*<>?|_!",
			"lol/_______________________________ _______!",
			"lol\\_______________________________ _______!"),

		// Allow UTF-8 characters.
		FilterCacheKeyTest_mode(
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE",
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE",
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE"),

		// Allow UTF-8 characters while filtering bad ASCII characters.
		FilterCacheKeyTest_mode(
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE\x01\x02",
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE__",
			"\xC2\xA9\xC2\xAE\xE2\x99\xAA\xE2\x98\x83\xF0\x9F\x92\xBE__"),

		// Disallow invalid UTF-8 sequences.
		// Reference: https://en.wikipedia.org/wiki/UTF-8
		// - Invalid sequence: \x80\xC0\xE0\xF0\xF8
		// - Overlong encoding: U+0000 -> \xC0\x80 (Modified UTF-8)
		// - Overlong encoding: U+0020 -> \xE0\x80\xA0
		// - Overlong encoding: U+20AC -> \xF0\x82\x82\xAC
		FilterCacheKeyTest_mode(
			"\xC2\xA9\x80\xC0\xE0\xF0\xF8\xC0\x80\xE0\x80\xA0\xF0\x82\x82\xAC",
			"\xC2\xA9______________",
			"\xC2\xA9______________")
	));
} }

/**
 * Test suite main function.
 */
extern "C" int gtest_main(int argc, TCHAR *argv[])
{
	fprintf(stderr, "LibCacheCommon test suite: LibCacheCommon::filterCacheKey() tests.\n\n");
	fflush(nullptr);

	// coverity[fun_call_w_exception]: uncaught exceptions cause nonzero exit anyway, so don't warn.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
