/***************************************************************************
 * ROM Properties Page shell extension. (librpbase/tests)                  *
 * TextFuncsTest.cpp: TextFuncs class test.                                *
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

// Google Test
#include "gtest/gtest.h"

// TextFuncs
#include "../TextFuncs.hpp"
#include "../byteorder.h"

#if !defined(RP_UTF8) && !defined(RP_UTF16)
#error Neither RP_UTF8 nor RP_UTF16 are defined.
#elif defined(RP_UTF8) && defined(RP_UTF16)
#error Both RP_UTF8 and RP_UTF16 are defined.
#endif

// C includes. (C++ namespace)
#include <cstdio>

// C++ includes.
#include <string>
using std::string;
using std::u16string;

// NOTE: We're redefining ARRAY_SIZE() here in order to
// get a size_t instead of an int.
#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif

/**
 * Number of elements in an array.
 *
 * Includes a static check for pointers to make sure
 * a dynamically-allocated array wasn't specified.
 * Reference: http://stackoverflow.com/questions/8018843/macro-definition-array-size
 */
#define ARRAY_SIZE(x) \
	(((sizeof(x) / sizeof(x[0]))) / \
		(size_t)(!(sizeof(x) % sizeof(x[0]))))

namespace LibRpBase { namespace Tests {

class TextFuncsTest : public ::testing::Test
{
	protected:
		TextFuncsTest() { }

	public:
		// NOTE: 8-bit test strings are unsigned in order to prevent
		// narrowing conversion warnings from appearing.
		// char16_t is defined as unsigned, so this isn't a problem
		// for 16-bit strings.

		/**
		 * cp1252 test string.
		 * Contains all possible cp1252 characters.
		 */
		static const uint8_t cp1252_data[250];

		/**
		 * cp1252 to UTF-8 test string.
		 * Contains the expected result from:
		 * - cp1252_to_utf8(cp1252_data, ARRAY_SIZE(cp1252_data))
		 * - cp1252_sjis_to_utf8(cp1252_data, ARRAY_SIZE(cp1252_data))
		 */
		static const uint8_t cp1252_utf8_data[388];

		/**
		 * cp1252 to UTF-16 test string.
		 * Contains the expected result from:
		 * - cp1252_to_utf16(cp1252_data, sizeof(cp1252_data))
		 * - cp1252_sjis_to_utf16(cp1252_data, sizeof(cp1252_data))
		 */
		static const char16_t cp1252_utf16_data[250];

		/**
		 * Shift-JIS test string.
		 *
		 * TODO: Get a longer test string.
		 * This string is from the JP Pokemon Colosseum (GCN) save file,
		 * plus a wave dash character (8160).
		 */
		static const uint8_t sjis_data[36];

		/**
		 * Shift-JIS to UTF-8 test string.
		 * Contains the expected result from:
		 * - cp1252_sjis_to_utf8(sjis_data, ARRAY_SIZE(sjis_data))
		 */
		static const uint8_t sjis_utf8_data[53];

		/**
		 * Shift-JIS to UTF-16 test string.
		 * Contains the expected result from:
		 * - cp1252_sjis_to_utf16(sjis_data, ARRAY_SIZE(sjis_data))
		 */
		static const char16_t sjis_utf16_data[19];

		/**
		 * Shift-JIS test string with a cp1252 copyright symbol. (0xA9)
		 * This string is incorrectly detected as Shift-JIS because
		 * all bytes are valid.
		 */
		static const uint8_t sjis_copyright_in[16];

		/**
		 * UTF-8 result from:
		 * - cp1252_sjis_to_utf8(sjis_copyright_in, sizeof(sjis_copyright_in))
		 */
		static const uint8_t sjis_copyright_out_utf8[18];

		/**
		 * UTF-16 result from:
		 * - cp1252_sjis_to_utf16(sjis_copyright_in, sizeof(sjis_copyright_in))
		 */
		static const char16_t sjis_copyright_out_utf16[16];

		/**
		 * UTF-8 test string.
		 * Contains Latin-1, BMP, and SMP characters.
		 *
		 * This contains the same string as
		 * utf16le_data[] and utf16be_data[].
		 */
		static const uint8_t utf8_data[325];

		/**
		 * UTF-16LE test string.
		 * Contains Latin-1, BMP, and SMP characters.
		 *
		 * This contains the same string as
		 * utf8_data[] and utf16be_data[].
		 *
		 * NOTE: This is encoded as uint8_t to prevent
		 * byteswapping issues.
		 */
		static const uint8_t utf16le_data[558];

		/**
		 * UTF-16BE test string.
		 * Contains Latin-1, BMP, and SMP characters.
		 *
		 * This contains the same string as
		 * utf8_data[] and utf16le_data[].
		 *
		 * NOTE: This is encoded as uint8_t to prevent
		 * byteswapping issues.
		 */
		static const uint8_t utf16be_data[558];

		// Host-endian UTF-16 data for functions
		// that convert to/from host-endian UTF-16.
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
		#define utf16_data utf16le_data
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
		#define utf16_data utf16be_data
#endif

		/**
		 * Latin1 to UTF-8 test string.
		 * Contains the expected result from:
		 * - latin1_to_utf8(cp1252_data, ARRAY_SIZE(cp1252_data))
		 * (NOTE: Unsupported characters are replaced with U+FFFD.)
		 */
		static const uint8_t latin1_utf8_data[346+(26*2)];

		/**
		 * Latin1 to UTF-16 test string.
		 * Contains the expected result from:
		 * - latin1_to_utf16(cp1252_data, ARRAY_SIZE(cp1252_data))
		 * (NOTE: Unsupported characters are replaced with U+FFFD.)
		 */
		static const char16_t latin1_utf16_data[250];
};

// Test strings are located in TextFuncsTest_data.hpp.
#include "TextFuncsTest_data.hpp"

/** Code Page 1252 **/

/**
 * Test cp1252_to_utf8().
 */
TEST_F(TextFuncsTest, cp1252_to_utf8)
{
	// Test with implicit length.
	string str = cp1252_to_utf8((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);

	// Test with explicit length.
	str = cp1252_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);
}

/**
 * Test cp1252_to_utf16().
 */
TEST_F(TextFuncsTest, cp1252_to_utf16)
{
	// Test with implicit length.
	u16string str = cp1252_to_utf16((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);

	// Test with explicit length.
	str = cp1252_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);
}

/** Code Page 1252 + Shift-JIS (932) **/

/**
 * Test cp1252_sjis_to_utf8() fallback functionality.
 * This string should be detected as cp1252 due to
 * Shift-JIS decoding errors.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf8_fallback)
{
	// Test with implicit length.
	string str = cp1252_sjis_to_utf8((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, str);
}

/**
 * Test cp1252_sjis_to_utf8() fallback functionality.
 * This string is incorrectly detected as Shift-JIS because
 * all bytes are valid.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf8_copyright)
{
	// cp1252 code point 0xA9 is the copyright symbol,
	// but it's also halfwidth katakana "U" in Shift-JIS.

	// Test with implicit length.
	string str = cp1252_sjis_to_utf8((const char*)sjis_copyright_in, -1);
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, str.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf8((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in)-1);
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, str.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf8((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in));
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, str.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, str);
}

/**
 * Test cp1252_sjis_to_utf8() with ASCII strings.
 * Note that backslashes will *not* be converted to
 * yen symbols, so this should be a no-op.
 *
 * FIXME: Backslash may be converted to yen symbols
 * on Windows if the system has a Japanese locale.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf8_ascii)
{
	static const char cp1252_in[] = "C:\\Windows\\System32";

	// Test with implicit length.
	string str = cp1252_sjis_to_utf8(cp1252_in, -1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, str.size());
	EXPECT_EQ(cp1252_in, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf8(cp1252_in, ARRAY_SIZE(cp1252_in)-1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, str.size());
	EXPECT_EQ(cp1252_in, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf8(cp1252_in, ARRAY_SIZE(cp1252_in));
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, str.size());
	EXPECT_EQ(cp1252_in, str);
}

/**
 * Test cp1252_sjis_to_utf8() with Japanese text.
 * This includes a wave dash character (8160).
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf8_japanese)
{
	// Test with implicit length.
	string str = cp1252_sjis_to_utf8((const char*)sjis_data, -1);
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)sjis_utf8_data, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf8((const char*)sjis_data, ARRAY_SIZE(sjis_data)-1);
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)sjis_utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf8((const char*)sjis_data, ARRAY_SIZE(sjis_data));
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)sjis_utf8_data, str);
}

/**
 * Test cp1252_sjis_to_utf16() fallback functionality.
 * This strings should be detected as cp1252 due to
 * Shift-JIS decoding errors.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf16_fallback)
{
	// Test with implicit length.
	u16string str = cp1252_sjis_to_utf16((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, str.size());
	EXPECT_EQ(cp1252_utf16_data, str);
}

/**
 * Test cp1252_sjis_to_utf8() fallback functionality.
 * This string is incorrectly detected as Shift-JIS because
 * all bytes are valid.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf16_copyright)
{
	// cp1252 code point 0xA9 is the copyright symbol,
	// but it's also halfwidth katakana "U" in Shift-JIS.

	// Test with implicit length.
	u16string str = cp1252_sjis_to_utf16((const char*)sjis_copyright_in, -1);
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, str.size());
	EXPECT_EQ(sjis_copyright_out_utf16, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf16((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in)-1);
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, str.size());
	EXPECT_EQ(sjis_copyright_out_utf16, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf16((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in));
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, str.size());
	EXPECT_EQ(sjis_copyright_out_utf16, str);
}

/**
 * Test cp1252_sjis_to_utf16() with ASCII strings.
 * Note that backslashes will *not* be converted to
 * yen symbols, so this should be a no-op.
 *
 * FIXME: Backslash may be converted to yen symbols
 * on Windows if the system has a Japanese locale.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf16_ascii)
{
	static const char cp1252_in[] = "C:\\Windows\\System32";

	// NOTE: Need to manually initialize the char16_t[] array
	// due to the way _RP() is implemented for versions of
	// MSVC older than 2015.
	// TODO: Hex and/or _RP_CHR()?
	static const char16_t utf16_out[] = {
		'C',':','\\','W','i','n','d','o',
		'w','s','\\','S','y','s','t','e',
		'm','3','2',0
	};

	// Test with implicit length.
	u16string str = cp1252_sjis_to_utf16(cp1252_in, -1);
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, str.size());
	EXPECT_EQ(utf16_out, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf16(cp1252_in, ARRAY_SIZE(cp1252_in)-1);
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, str.size());
	EXPECT_EQ(utf16_out, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf16(cp1252_in, ARRAY_SIZE(cp1252_in));
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, str.size());
	EXPECT_EQ(utf16_out, str);
}

/**
 * Test cp1252_sjis_to_utf16() with Japanese text.
 * This includes a wave dash character (8160).
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_utf16_japanese)
{
	// Test with implicit length.
	u16string str = cp1252_sjis_to_utf16((const char*)sjis_data, -1);
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, str.size());
	EXPECT_EQ(sjis_utf16_data, str);

	// Test with explicit length.
	str = cp1252_sjis_to_utf16((const char*)sjis_data, ARRAY_SIZE(sjis_data)-1);
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, str.size());
	EXPECT_EQ(sjis_utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = cp1252_sjis_to_utf16((const char*)sjis_data, ARRAY_SIZE(sjis_data));
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, str.size());
	EXPECT_EQ(sjis_utf16_data, str);
}

/** UTF-8 to UTF-16 and vice-versa **/

/**
 * Test utf8_to_utf16() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf8_to_utf16)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	u16string str = utf8_to_utf16((const char*)utf8_data, -1);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);

	// Test with explicit length.
	str = utf8_to_utf16((const char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = utf8_to_utf16((const char*)utf8_data, ARRAY_SIZE(utf8_data));
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);
}

/**
 * Test utf16le_to_utf8() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf16le_to_utf8)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	string str = utf16le_to_utf8((const char16_t*)utf16le_data, -1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length.
	str = utf16le_to_utf8((const char16_t*)utf16le_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = utf16le_to_utf8((const char16_t*)utf16le_data, (sizeof(utf16_data)/sizeof(char16_t)));
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);
}

/**
 * Test utf16be_to_utf8() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf16be_to_utf8)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	string str = utf16be_to_utf8((const char16_t*)utf16be_data, -1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length.
	str = utf16be_to_utf8((const char16_t*)utf16be_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = utf16be_to_utf8((const char16_t*)utf16be_data, (sizeof(utf16_data)/sizeof(char16_t)));
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);
}

/**
 * Test utf16_to_utf8() with regular text and special characters.
 * NOTE: This is effectively the same as the utf16le_to_utf8()
 * or utf16be_to_utf8() test, depending on system architecture.
 * This test ensures the byteorder macros are working correctly.
 */
TEST_F(TextFuncsTest, utf16_to_utf8)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	string str = utf16_to_utf8((const char16_t*)utf16_data, -1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length.
	str = utf16_to_utf8((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = utf16_to_utf8((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t)));
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);
}

/**
 * Test utf16_bswap() with regular text and special characters.
 * This function converts from BE to LE.
 */
TEST_F(TextFuncsTest, utf16_bswap_BEtoLE)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	u16string str = utf16_bswap((const char16_t*)utf16be_data, -1);
	EXPECT_EQ((sizeof(utf16le_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16le_data, str);

	// Test with explicit length.
	str = utf16_bswap((const char16_t*)utf16be_data, (sizeof(utf16be_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16le_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16le_data, str);

	// Test with explicit length and an extra NULL.
	// NOTE: utf16_bswap does NOT trim NULLs.
	str = utf16_bswap((const char16_t*)utf16be_data, (sizeof(utf16be_data)/sizeof(char16_t)));
	EXPECT_EQ((sizeof(utf16le_data)/sizeof(char16_t)), str.size());
	// Remove the extra NULL before comparing.
	str.resize(str.size()-1);
	EXPECT_EQ((const char16_t*)utf16le_data, str);
}

/**
 * Test utf16_bswap() with regular text and special characters.
 * This function converts from LE to BE.
 */
TEST_F(TextFuncsTest, utf16_bswap_LEtoBE)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	u16string str = utf16_bswap((const char16_t*)utf16le_data, -1);
	EXPECT_EQ((sizeof(utf16be_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16be_data, str);

	// Test with explicit length.
	str = utf16_bswap((const char16_t*)utf16le_data, (sizeof(utf16le_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16be_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16be_data, str);

	// Test with explicit length and an extra NULL.
	// NOTE: utf16_bswap does NOT trim NULLs.
	str = utf16_bswap((const char16_t*)utf16le_data, (sizeof(utf16le_data)/sizeof(char16_t)));
	EXPECT_EQ((sizeof(utf16be_data)/sizeof(char16_t)), str.size());
	// Remove the extra NULL before comparing.
	str.resize(str.size()-1);
	EXPECT_EQ((const char16_t*)utf16be_data, str);
}

/** Latin-1 (ISO-8859-1) **/

/**
 * Test latin1_to_utf8().
 */
TEST_F(TextFuncsTest, latin1_to_utf8)
{
	// Test with implicit length.
	string str = latin1_to_utf8((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)latin1_utf8_data, str);

	// Test with explicit length.
	str = latin1_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)latin1_utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = latin1_to_utf8((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, str.size());
	EXPECT_EQ((const char*)latin1_utf8_data, str);
}

/**
 * Test latin1_to_utf16().
 */
TEST_F(TextFuncsTest, latin1_to_utf16)
{
	// Test with implicit length.
	u16string str = latin1_to_utf16((const char*)cp1252_data, -1);
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, str.size());
	EXPECT_EQ(latin1_utf16_data, str);

	// Test with explicit length.
	str = latin1_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, str.size());
	EXPECT_EQ(latin1_utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	str = latin1_to_utf16((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, str.size());
	EXPECT_EQ(latin1_utf16_data, str);
}

/** Miscellaneous functions. **/

/**
 * Test u16_strlen().
 */
TEST_F(TextFuncsTest, u16_strlen)
{
	// NOTE: u16_strlen() is a wrapper for wcslen() on Windows.
	// On all other systems, it's a simple implementation.

	// Compare to 8-bit strlen() with ASCII.
	static const char ascii_in[] = "abcdefghijklmnopqrstuvwxyz";
	static const char16_t u16_in[] = {
		'a','b','c','d','e','f','g','h','i','j','k','l',
		'm','n','o','p','q','r','s','t','u','v','w','x',
		'y','z',0
	};

	EXPECT_EQ(ARRAY_SIZE(ascii_in)-1, strlen(ascii_in));
	EXPECT_EQ(ARRAY_SIZE(u16_in)-1, u16_strlen(u16_in));
	EXPECT_EQ(strlen(ascii_in), u16_strlen(u16_in));

	// Test u16_strlen() with SMP characters.
	// u16_strlen() will return the number of 16-bit characters,
	// NOT the number of code points.
	static const char16_t u16smp_in[] = {
		0xD83C,0xDF4C,0xD83C,0xDF59,
		0xD83C,0xDF69,0xD83D,0xDCB5,
		0xD83D,0xDCBE,0x0000
	};
	EXPECT_EQ(ARRAY_SIZE(u16smp_in)-1, u16_strlen(u16smp_in));
}

/**
 * Test u16_strdup().
 */
TEST_F(TextFuncsTest, u16_strdup)
{
	// NOTE: u16_strdup() is a wrapper for wcsdup() on Windows.
	// On all other systems, it's a simple implementation.

	// Test string.
	static const char16_t u16_str[] = {
		'T','h','e',' ','q','u','i','c','k',' ','b','r',
		'o','w','n',' ','f','o','x',' ','j','u','m','p',
		's',' ','o','v','e','r',' ','t','h','e',' ','l',
		'a','z','y',' ','d','o','g','.',0
	};

	char16_t *u16_dup = u16_strdup(u16_str);
	ASSERT_TRUE(u16_dup != nullptr);

	// Verify the NULL terminator.
	EXPECT_EQ(0, u16_str[ARRAY_SIZE(u16_str)-1]);
	if (u16_str[ARRAY_SIZE(u16_str)-1] != 0) {
		// NULL terminator not found.
		// u16_strlen() and u16_strcmp() may crash,
		// so exit early.
		// NOTE: We can't use ASSERT_EQ() because we
		// have to free the u16_strdup()'d string.
		free(u16_dup);
		return;
	}

	// Verify the string length.
	EXPECT_EQ(ARRAY_SIZE(u16_str)-1, u16_strlen(u16_dup));

	// Verify the string contents.
	// NOTE: EXPECT_STREQ() supports const wchar_t*,
	// but not const char16_t*.
	EXPECT_EQ(0, u16_strcmp(u16_str, u16_dup));

	free(u16_dup);
}

/**
 * Test u16_strcmp().
 */
TEST_F(TextFuncsTest, u16_strcmp)
{
	// NOTE: u16_strcmp() is a wrapper for wcscmp() on Windows.
	// On all other systems, it's a simple implementation.

	// Three test strings.
	static const char16_t u16_str1[] = {'a','b','c',0};
	static const char16_t u16_str2[] = {'a','b','d',0};
	static const char16_t u16_str3[] = {'d','e','f',0};

	// Compare strings to themselves.
	EXPECT_EQ(0, u16_strcmp(u16_str1, u16_str1));
	EXPECT_EQ(0, u16_strcmp(u16_str2, u16_str2));
	EXPECT_EQ(0, u16_strcmp(u16_str3, u16_str3));

	// Compare strings to each other.
	EXPECT_LT(u16_strcmp(u16_str1, u16_str2), 0);
	EXPECT_LT(u16_strcmp(u16_str1, u16_str3), 0);
	EXPECT_GT(u16_strcmp(u16_str2, u16_str1), 0);
	EXPECT_LT(u16_strcmp(u16_str2, u16_str3), 0);
	EXPECT_GT(u16_strcmp(u16_str3, u16_str1), 0);
	EXPECT_GT(u16_strcmp(u16_str3, u16_str2), 0);
}

/** rp_string wrappers. **/
// These functions depend on the libromdata build type.
// NOTE: Most of these tests are copied from the above
// tests, but have been modified to use rp_string.

/**
 * Test cp1252_to_rp_string().
 */
TEST_F(TextFuncsTest, cp1252_to_rp_string)
{
	// Test with implicit length.
	rp_string rps = cp1252_to_rp_string((const char*)cp1252_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif

	// Test with explicit length.
	rps = cp1252_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = cp1252_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif
}

/**
 * Test cp1252_sjis_to_rp_string() fallback functionality.
 * This string should be detected as cp1252 due to
 * Shift-JIS decoding errors.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_rp_string_fallback)
{
	// Test with implicit length.
	rp_string rps = cp1252_sjis_to_rp_string((const char*)cp1252_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif

	// Test with explicit length.
	rps = cp1252_sjis_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = cp1252_sjis_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)cp1252_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(cp1252_utf16_data)-1, rps.size());
	EXPECT_EQ(cp1252_utf16_data, rps);
#endif
}

/**
 * Test cp1252_sjis_to_rp_string() fallback functionality.
 * This string is incorrectly detected as Shift-JIS because
 * all bytes are valid.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_rp_string_copyright)
{
	// cp1252 code point 0xA9 is the copyright symbol,
	// but it's also halfwidth katakana "U" in Shift-JIS.

	// Test with implicit length.
	rp_string rps = cp1252_sjis_to_rp_string((const char*)sjis_copyright_in, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, rps.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, rps.size());
	EXPECT_EQ(sjis_copyright_out_utf16, rps);
#endif

	// Test with explicit length.
	rps = cp1252_sjis_to_rp_string((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, rps.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, rps.size());
	EXPECT_EQ(sjis_copyright_out_utf16, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = cp1252_sjis_to_rp_string((const char*)sjis_copyright_in, ARRAY_SIZE(sjis_copyright_in));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf8)-1, rps.size());
	EXPECT_EQ((const char*)sjis_copyright_out_utf8, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_copyright_out_utf16)-1, rps.size());
	EXPECT_EQ(sjis_copyright_out_utf16, rps);
#endif
}

/**
 * Test cp1252_sjis_to_rp_string() with ASCII strings.
 * Note that backslashes will *not* be converted to
 * yen symbols, so this should be a no-op.
 *
 * FIXME: Backslash may be converted to yen symbols
 * on Windows if the system has a Japanese locale.
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_rp_string_ascii)
{
	static const char cp1252_in[] = "C:\\Windows\\System32";
#ifdef RP_UTF16
	// NOTE: Need to manually initialize the char16_t[] array
	// due to the way _RP() is implemented for versions of
	// MSVC older than 2015.
	// TODO: Hex and/or _RP_CHR()?
	static const char16_t utf16_out[] = {
		'C',':','\\','W','i','n','d','o',
		'w','s','\\','S','y','s','t','e',
		'm','3','2',0
	};
#endif

	// Test with implicit length.
	rp_string rps = cp1252_sjis_to_rp_string(cp1252_in, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, rps.size());
	EXPECT_EQ(cp1252_in, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, rps.size());
	EXPECT_EQ(utf16_out, rps);
#endif

	// Test with explicit length.
	rps = cp1252_sjis_to_rp_string(cp1252_in, ARRAY_SIZE(cp1252_in)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, rps.size());
	EXPECT_EQ(cp1252_in, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, rps.size());
	EXPECT_EQ(utf16_out, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = cp1252_sjis_to_rp_string(cp1252_in, ARRAY_SIZE(cp1252_in));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(cp1252_in)-1, rps.size());
	EXPECT_EQ(cp1252_in, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(utf16_out)-1, rps.size());
	EXPECT_EQ(utf16_out, rps);
#endif
}

/**
 * Test cp1252_sjis_to_rp_string() with Japanese text.
 * This includes a wave dash character (8160).
 */
TEST_F(TextFuncsTest, cp1252_sjis_to_rp_string_japanese)
{
	// Test with implicit length.
	rp_string rps = cp1252_sjis_to_rp_string((const char*)sjis_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)sjis_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, rps.size());
	EXPECT_EQ(sjis_utf16_data, rps);
#endif

	// Test with explicit length.
	rps = cp1252_sjis_to_rp_string((const char*)sjis_data, ARRAY_SIZE(sjis_data)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)sjis_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, rps.size());
	EXPECT_EQ(sjis_utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = cp1252_sjis_to_rp_string((const char*)sjis_data, ARRAY_SIZE(sjis_data));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(sjis_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)sjis_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(sjis_utf16_data)-1, rps.size());
	EXPECT_EQ(sjis_utf16_data, rps);
#endif
}

/**
 * Test utf8_to_rp_string() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf8_to_rp_string)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	rp_string rps = utf8_to_rp_string((const char*)utf8_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length.
	rps = utf8_to_rp_string((const char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = utf8_to_rp_string((const char*)utf8_data, ARRAY_SIZE(utf8_data));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// utf8_to_rp_string(const std::string &str) test.
	string str((const char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	rps = utf8_to_rp_string(str);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif
}

/**
 * Test rp_string_to_utf8() with regular text and special characters.
 */
TEST_F(TextFuncsTest, rp_string_to_utf8)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
#ifdef RP_UTF8
	string str = rp_string_to_utf8((const rp_char*)utf8_data, -1);
#else
	string str = rp_string_to_utf8((const rp_char*)utf16_data, -1);
#endif
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length.
#ifdef RP_UTF8
	str = rp_string_to_utf8((const rp_char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
#else
	str = rp_string_to_utf8((const rp_char*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
#endif
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
#ifdef RP_UTF8
	str = rp_string_to_utf8((const rp_char*)utf8_data, ARRAY_SIZE(utf8_data));
#else
	str = rp_string_to_utf8((const rp_char*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t)));
#endif
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);

	// rp_string_to_utf8(const rp_string &rps) test.
#ifdef RP_UTF8
	rp_string rps((const char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
#else /* RP_UTF16 */
	rp_string rps((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
#endif
	str = rp_string_to_utf8(rps);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, str.size());
	EXPECT_EQ((const char*)utf8_data, str);
}

/**
 * Test utf16le_to_rp_string() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf16le_to_rp_string)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	rp_string rps = utf16le_to_rp_string((const char16_t*)utf16le_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length.
	rps = utf16le_to_rp_string((const char16_t*)utf16le_data, (sizeof(utf16le_data)/sizeof(char16_t))-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = utf16le_to_rp_string((const char16_t*)utf16le_data, (sizeof(utf16le_data)/sizeof(char16_t)));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// utf16le_to_rp_string(const std::u16string &str) test.
	u16string str((const char16_t*)utf16le_data, (sizeof(utf16le_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16le_data)/sizeof(char16_t))-1, str.size());
	rps = utf16le_to_rp_string(str);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif
}

/**
 * Test utf16be_to_rp_string() with regular text and special characters.
 */
TEST_F(TextFuncsTest, utf16be_to_rp_string)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	rp_string rps = utf16be_to_rp_string((const char16_t*)utf16be_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length.
	rps = utf16be_to_rp_string((const char16_t*)utf16be_data, (sizeof(utf16be_data)/sizeof(char16_t))-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = utf16be_to_rp_string((const char16_t*)utf16be_data, (sizeof(utf16be_data)/sizeof(char16_t)));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// utf16be_to_rp_string(const std::u16string &str) test.
	u16string str((const char16_t*)utf16be_data, (sizeof(utf16be_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16be_data)/sizeof(char16_t))-1, str.size());
	rps = utf16be_to_rp_string(str);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif
}

/**
 * Test utf16_to_rp_string() with regular text and special characters.
 * NOTE: This is effectively the same as the utf16le_to_rp_string()
 * or utf16be_to_rp_string() test, depending on system architecture.
 * This test ensures the byteorder macros are working correctly.
 */
TEST_F(TextFuncsTest, utf16_to_rp_string)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
	rp_string rps = utf16_to_rp_string((const char16_t*)utf16_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length.
	rps = utf16_to_rp_string((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = utf16_to_rp_string((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t)));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif

	// utf16_to_rp_string(const std::u16string &str) test.
	u16string str((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	rps = utf16_to_rp_string(str);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
	EXPECT_EQ((const char16_t*)utf16_data, rps);
#endif
}

/**
 * Test rp_string_to_utf16() with regular text and special characters.
 */
TEST_F(TextFuncsTest, rp_string_to_utf16)
{
	// NOTE: The UTF-16 test strings are stored as
	// uint8_t arrays in order to prevent byteswapping
	// by the compiler.

	// Test with implicit length.
#ifdef RP_UTF8
	u16string str = rp_string_to_utf16((const rp_char*)utf8_data, -1);
#else
	u16string str = rp_string_to_utf16((const rp_char*)utf16_data, -1);
#endif
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);

	// Test with explicit length.
#ifdef RP_UTF8
	str = rp_string_to_utf16((const rp_char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
#else
	str = rp_string_to_utf16((const rp_char*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
#endif
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
#ifdef RP_UTF8
	str = rp_string_to_utf16((const rp_char*)utf8_data, ARRAY_SIZE(utf8_data));
#else
	str = rp_string_to_utf16((const rp_char*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t)));
#endif
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);

	// rp_string_to_utf16(const rp_string &rps) test.
#ifdef RP_UTF8
	rp_string rps((const char*)utf8_data, ARRAY_SIZE(utf8_data)-1);
	EXPECT_EQ(ARRAY_SIZE(utf8_data)-1, rps.size());
#else /* RP_UTF16 */
	rp_string rps((const char16_t*)utf16_data, (sizeof(utf16_data)/sizeof(char16_t))-1);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, rps.size());
#endif
	str = rp_string_to_utf16(rps);
	EXPECT_EQ((sizeof(utf16_data)/sizeof(char16_t))-1, str.size());
	EXPECT_EQ((const char16_t*)utf16_data, str);
}

/**
 * Test latin1_to_rp_string().
 */
TEST_F(TextFuncsTest, latin1_to_rp_string)
{
	// Test with implicit length.
	rp_string rps = latin1_to_rp_string((const char*)cp1252_data, -1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)latin1_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, rps.size());
	EXPECT_EQ(latin1_utf16_data, rps);
#endif

	// Test with explicit length.
	rps = latin1_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data)-1);
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)latin1_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, rps.size());
	EXPECT_EQ(latin1_utf16_data, rps);
#endif

	// Test with explicit length and an extra NULL.
	// The extra NULL should be trimmed.
	rps = latin1_to_rp_string((const char*)cp1252_data, ARRAY_SIZE(cp1252_data));
#ifdef RP_UTF8
	EXPECT_EQ(ARRAY_SIZE(latin1_utf8_data)-1, rps.size());
	EXPECT_EQ((const char*)latin1_utf8_data, rps);
#else /* RP_UTF16 */
	EXPECT_EQ(ARRAY_SIZE(latin1_utf16_data)-1, rps.size());
	EXPECT_EQ(latin1_utf16_data, rps);
#endif
}

/**
 * Test rp_strlen().
 */
TEST_F(TextFuncsTest, rp_strlen)
{
	// Compare to 8-bit strlen() with ASCII.
	static const char ascii_in[] = "abcdefghijklmnopqrstuvwxyz";
	static const rp_char rpc_in[] = {
		'a','b','c','d','e','f','g','h','i','j','k','l',
		'm','n','o','p','q','r','s','t','u','v','w','x',
		'y','z',0
	};

	EXPECT_EQ(ARRAY_SIZE(ascii_in)-1, strlen(ascii_in));
	EXPECT_EQ(ARRAY_SIZE(rpc_in)-1, rp_strlen(rpc_in));
	EXPECT_EQ(strlen(ascii_in), rp_strlen(rpc_in));

	// SMP test skipped for rp_strlen().
	// If it worked iwth u16_strlen(), it'll work with
	// rp_strlen() in RP_UTF16, and in RP_UTF8,
	// it'll be strlen().
}

/**
 * Test rp_strdup().
 */
TEST_F(TextFuncsTest, rp_strdup)
{
	// Test string.
	static const rp_char rpc_str[] = {
		'T','h','e',' ','q','u','i','c','k',' ','b','r',
		'o','w','n',' ','f','o','x',' ','j','u','m','p',
		's',' ','o','v','e','r',' ','t','h','e',' ','l',
		'a','z','y',' ','d','o','g','.',0
	};

	rp_char *rpc_dup = rp_strdup(rpc_str);
	ASSERT_TRUE(rpc_dup != nullptr);

	// Verify the NULL terminator.
	EXPECT_EQ(0, rpc_str[ARRAY_SIZE(rpc_str)-1]);
	if (rpc_str[ARRAY_SIZE(rpc_str)-1] != 0) {
		// NULL terminator not found.
		// u16_strlen() and u16_strcmp() may crash,
		// so exit early.
		// NOTE: We can't use ASSERT_EQ() because we
		// have to free the u16_strdup()'d string.
		free(rpc_dup);
		return;
	}

	// Verify the string length.
	EXPECT_EQ(ARRAY_SIZE(rpc_str)-1, rp_strlen(rpc_dup));

	// Verify the string contents.
	// NOTE: EXPECT_STREQ() supports const wchar_t*,
	// but not const char16_t*.
	EXPECT_EQ(0, rp_strcmp(rpc_str, rpc_dup));
	free(rpc_dup);

	// Test the rp_string overload.
	rp_string rps(rpc_str, ARRAY_SIZE(rpc_str)-1);
	EXPECT_EQ(rpc_str, rps);
	rpc_dup = rp_strdup(rps);
	ASSERT_TRUE(rpc_dup != nullptr);
	EXPECT_EQ(rps, rpc_dup);
	free(rpc_dup);
}

/**
 * Test u16_strcmp().
 */
TEST_F(TextFuncsTest, rp_strcmp)
{
	// Three test strings.
	static const rp_char rpc_str1[] = {'a','b','c',0};
	static const rp_char rpc_str2[] = {'a','b','d',0};
	static const rp_char rpc_str3[] = {'d','e','f',0};

	// Compare strings to themselves.
	EXPECT_EQ(0, rp_strcmp(rpc_str1, rpc_str1));
	EXPECT_EQ(0, rp_strcmp(rpc_str2, rpc_str2));
	EXPECT_EQ(0, rp_strcmp(rpc_str3, rpc_str3));

	// Compare strings to each other.
	EXPECT_LT(rp_strcmp(rpc_str1, rpc_str2), 0);
	EXPECT_LT(rp_strcmp(rpc_str1, rpc_str3), 0);
	EXPECT_GT(rp_strcmp(rpc_str2, rpc_str1), 0);
	EXPECT_LT(rp_strcmp(rpc_str2, rpc_str3), 0);
	EXPECT_GT(rp_strcmp(rpc_str3, rpc_str1), 0);
	EXPECT_GT(rp_strcmp(rpc_str3, rpc_str2), 0);
}

} }

/**
 * Test suite main function.
 */
extern "C" int gtest_main(int argc, char *argv[])
{
	fprintf(stderr, "LibRpBase test suite: TextFuncs tests.\n\n");
	fflush(nullptr);

	// coverity[fun_call_w_exception]: uncaught exceptions cause nonzero exit anyway, so don't warn.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
