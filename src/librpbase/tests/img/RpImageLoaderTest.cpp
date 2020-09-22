/***************************************************************************
 * ROM Properties Page shell extension. (librpbase/tests)                  *
 * RpImageLoaderTest.cpp: RpImageLoader class test.                        *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

// Google Test
#include "gtest/gtest.h"
#include "tcharx.h"

// zlib
#include <zlib.h>

// C includes. (C++ namespace)
#include <cstdio>

/**
 * Test suite main function.
 * Called by gtest_init.c.
 */
extern "C" int gtest_main(int argc, TCHAR *argv[])
{
	fprintf(stderr, "LibRpBase test suite: RpImageLoader tests.\n\n");
	fflush(nullptr);

	// Make sure the CRC32 table is initialized.
	get_crc_table();

	// coverity[fun_call_w_exception]: uncaught exceptions cause nonzero exit anyway, so don't warn.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
