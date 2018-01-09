/***************************************************************************
 * ROM Properties Page shell extension. (libromdata/tests)                 *
 * GcnFstTest.cpp: GameCube/Wii FST test.                                  *
 *                                                                         *
 * Copyright (c) 2016-2017 by David Korth.                                 *
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

// MiniZip
#include "mz_zip.h"
#include "mz_compat.h"

// librpbase
#include "librpbase/TextFuncs.hpp"
#include "librpbase/TextFuncs_utf8.hpp"
#include "librpbase/file/FileSystem.hpp"
using namespace LibRpBase;

// libromdata
#include "disc/GcnFst.hpp"
using LibRomData::GcnFst;

// libwin32common
#ifdef _WIN32
#include "libwin32common/RpWin32_sdk.h"
#endif

// FST printer.
#include "FstPrint.hpp"

// C includes. (C++ namespace)
#include <cctype>

// C++ includes.
#include <sstream>
#include <string>
#include <unordered_set>
using std::istringstream;
using std::ostream;
using std::string;
using std::stringstream;
using std::unordered_set;

// Uninitialized vector class.
// Reference: http://andreoffringa.org/?q=uvector
#include "uvector.h"

namespace LibRomData { namespace Tests {

struct GcnFstTest_mode
{
	std::string fst_filename;	// FST filename in the FST Zip file.
	uint8_t offsetShift;		// File offset shift. (0 == GCN, 2 == Wii)

	GcnFstTest_mode(
		const char *fst_filename,
		uint8_t offsetShift)
		: fst_filename(fst_filename)
		, offsetShift(offsetShift)
	{ }

	GcnFstTest_mode(
		const string &fst_filename,
		uint8_t offsetShift)
		: fst_filename(fst_filename)
		, offsetShift(offsetShift)
	{ }
};

/**
 * Formatting function for GcnFstTest_mode.
 */
inline ::std::ostream& operator<<(::std::ostream& os, const GcnFstTest_mode& mode) {
        return os << mode.fst_filename;
};

// Maximum file size for FST files.
static const int MAX_GCN_FST_BIN_FILESIZE = 1024*1024;	// 1.0 MB
static const int MAX_GCN_FST_TXT_FILESIZE = 1536*1024;	// 1.5 MB

class GcnFstTest : public ::testing::TestWithParam<GcnFstTest_mode>
{
	protected:
		GcnFstTest()
			: ::testing::TestWithParam<GcnFstTest_mode>()
			, m_fst(nullptr)
		{ }

		virtual void SetUp(void) override final;
		virtual void TearDown(void) override final;

		/**
		 * Open a Zip file for reading.
		 * @param filename Zip filename.
		 * @return Zip file, or nullptr on error.
		 */
		static unzFile openZip(const char *filename);

		/**
		 * Get a file from a Zip file.
		 * @param zip_filename	[in] Zip filename.
		 * @param int_filename	[in] Internal filename.
		 * @param buf		[out] uvector buffer for the data.
		 * @param max_filesize	[in,opt] Maximum file size. (default is MAX_GCN_FST_BIN_FILESIZE)
		 * @return Number of bytes read, or negative on error.
		 */
		static int getFileFromZip(const char *zip_filename,
			const char *int_filename,
			ao::uvector<uint8_t>& buf,
			int max_filesize = MAX_GCN_FST_BIN_FILESIZE);

	public:
		// FST data.
		ao::uvector<uint8_t> m_fst_buf;
		IFst *m_fst;

		/**
		 * Recursively check a subdirectory for duplicate filenames.
		 * @param subdir Subdirectory path.
		 */
		void checkNoDuplicateFilenames(const char *subdir);

		/**
		 * Print a uint32_t using en_US formatting.
		 * This is needed in order to work around issues
		 * caused by locales with number formatting that
		 * doesn't match en_US.
		 * @param os	[out] ostream.
		 * @param val	[in] Number.
		 */
		static void print_uint32_en_US(ostream &os, uint32_t val);

	public:
		/** Test case parameters. **/

		/**
		 
		 * Get the list of FST files from a Zip file.
		 * @param offsetShift File offset shift. (0 == GCN, 2 == Wii)
		 * @return FST files.
		 */
		static std::vector<GcnFstTest_mode> ReadTestCasesFromDisk(uint8_t offsetShift);

		/**
		 * Test case suffix generator.
		 * @param info Test parameter information.
		 * @return Test case suffix.
		 */
		static string test_case_suffix_generator(const ::testing::TestParamInfo<GcnFstTest_mode> &info);
};

/**
 * SetUp() function.
 * Run before each test.
 */
void GcnFstTest::SetUp(void)
{
	// Parameterized test.
	const GcnFstTest_mode &mode = GetParam();

	// Open the Zip file.
	// NOTE: MiniZip 2.2.3's compatibility functions
	// take UTF-8 on Windows, not UTF-16.
	const char *zip_filename = nullptr;
	switch (mode.offsetShift) {
		case 0:
			zip_filename = "GameCube.fst.bin.zip";
			break;
		case 2:
			zip_filename = "Wii.fst.bin.zip";
			break;
		default:
			ASSERT_TRUE(false) << "offsetShift is " << (int)mode.offsetShift << "; should be either 0 or 2.";
	}

	ASSERT_GT(getFileFromZip(zip_filename, mode.fst_filename.c_str(), m_fst_buf, MAX_GCN_FST_BIN_FILESIZE), 0);

	// Create the GcnFst object.
	m_fst = new GcnFst(m_fst_buf.data(), (uint32_t)m_fst_buf.size(), mode.offsetShift);
	ASSERT_TRUE(m_fst->isOpen());
}

/**
 * TearDown() function.
 * Run after each test.
 */
void GcnFstTest::TearDown(void)
{
	delete m_fst;
	m_fst = nullptr;
}

/**
 * Open a Zip file for reading.
 * @param filename Zip filename.
 * @return Zip file, or nullptr on error.
 */
unzFile GcnFstTest::openZip(const char *filename)
{
	// Prepend fst_data.
	string path = "fst_data";
	path += DIR_SEP_CHR;
	path += filename;

#ifdef _WIN32
	// NOTE: MiniZip 2.2.3's compatibility functions
	// take UTF-8 on Windows, not UTF-16.
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64W(&ffunc);
	return unzOpen2_64(path.c_str(), &ffunc);
#else /* !_WIN32 */
#ifdef RP_UTF8
	return unzOpen(path.c_str());
#else /* RP_UTF16 */
	return unzOpen(path.c_str());
#endif
#endif /* _WIN32 */
}

/**
 * Get a file from a Zip file.
 * @param zip_filename	[in] Zip filename.
 * @param int_filename	[in] Internal filename.
 * @param buf		[out] uvector buffer for the data.
 * @param max_filesize	[in,opt] Maximum file size. (default is MAX_GCN_FST_BIN_FILESIZE)
 * @return Number of bytes read, or negative on error.
 */
int GcnFstTest::getFileFromZip(const char *zip_filename,
	const char *int_filename,
	ao::uvector<uint8_t>& buf,
	int max_filesize)
{
	// Open the Zip file.
	// NOTE: MiniZip 2.2.3's compatibility functions
	// take UTF-8 on Windows, not UTF-16.
	unzFile unz = openZip(zip_filename);
	EXPECT_TRUE(unz != nullptr) <<
		"Could not open '" << zip_filename << "', check the test directory!";
	if (!unz) {
		return -1;
	}

	// Locate the required FST file.
	// TODO: Always case-insensitive? (currently using OS-dependent value)
	int ret = unzLocateFile(unz, int_filename, 0);
	EXPECT_EQ(UNZ_OK, ret);
	if (ret != UNZ_OK) {
		unzClose(unz);
		return -2;
	}

	// Verify the FST file size.
	unz_file_info64 file_info;
	ret = unzGetCurrentFileInfo64(unz,
		&file_info,
		nullptr, 0,
		nullptr, 0,
		nullptr, 0);
	EXPECT_EQ(UNZ_OK, ret);
	if (ret != UNZ_OK) {
		unzClose(unz);
		return -3;
	}
	EXPECT_LE(file_info.uncompressed_size, (uLong)max_filesize) <<
		"Compressed file '" << int_filename << "' is too big.";
	if (file_info.uncompressed_size > (uLong)max_filesize) {
		unzClose(unz);
		return -4;
	}

	// Open the FST file.
	ret = unzOpenCurrentFile(unz);
	EXPECT_EQ(UNZ_OK, ret);
	if (ret != UNZ_OK) {
		unzClose(unz);
		return -5;
	}

	// Read the FST file.
	// NOTE: zlib and minizip are only guaranteed to be able to
	// read UINT16_MAX (64 KB) at a time, and the updated MiniZip
	// from https://github.com/nmoinvaz/minizip enforces this.
	buf.resize((size_t)file_info.uncompressed_size);
	uint8_t *p = buf.data();
	size_t size = buf.size();
	while (size > 0) {
		int to_read = (int)(size > UINT16_MAX ? UINT16_MAX : size);
		ret = unzReadCurrentFile(unz, p, to_read);
		EXPECT_EQ(to_read, ret);
		if (ret != to_read) {
			unzClose(unz);
			return -6;
		}

		// ret == number of bytes read.
		p += ret;
		size -= ret;
	}

	// Read one more byte to ensure unzReadCurrentFile() returns 0 for EOF.
	// NOTE: MiniZip will zero out tmp if there's no data available.
	uint8_t tmp;
	ret = unzReadCurrentFile(unz, &tmp, 1);
	EXPECT_EQ(0, ret);
	if (ret != 0) {
		unzClose(unz);
		return -7;
	}

	// Close the FST file.
	// An error will occur here if the CRC is incorrect.
	ret = unzCloseCurrentFile(unz);
	EXPECT_EQ(UNZ_OK, ret);
	if (ret != UNZ_OK) {
		unzClose(unz);
		return -8;
	}

	// Close the Zip file.
	unzClose(unz);
	return (int)file_info.uncompressed_size;
}

/**
 * Recursively check a subdirectory for duplicate filenames.
 * @param subdir Subdirectory path.
 */
void GcnFstTest::checkNoDuplicateFilenames(const char *subdir)
{
	unordered_set<string> filenames;
	unordered_set<string> subdirs;

	IFst::Dir *dirp = m_fst->opendir(subdir);
	ASSERT_TRUE(dirp != nullptr) <<
		"Failed to open directory '" << subdir << "'.";

	IFst::DirEnt *dirent = m_fst->readdir(dirp);
	while (dirent != nullptr) {
		// Make sure we haven't seen this filename in
		// the current subdirectory yet.
		EXPECT_TRUE(filenames.find(dirent->name) == filenames.end()) <<
			"Directory '" << subdir << "' has duplicate filename '" << dirent->name << "'.";

		// Filename has been seen now.
		filenames.insert(dirent->name);

		// Is this a subdirectory?
		if (dirent->type == DT_DIR) {
			subdirs.insert(dirent->name);
		}

		// Next entry.
		dirent = m_fst->readdir(dirp);
	}

	// Check subdirectories.
	for (auto iter = subdirs.cbegin(); iter != subdirs.cend(); ++iter) {
		string path = subdir;
		if (!path.empty() && path[path.size()-1] != '/') {
			path += '/';
		}
		path += iter->c_str();
		checkNoDuplicateFilenames(path.c_str());
	}

	// End of directory.
	m_fst->closedir(dirp);
}

/**
 * Print a uint32_t using en_US formatting.
 * This is needed in order to work around issues
 * caused by locales with number formatting that
 * doesn't match en_US.
 * @param os	[out] ostream.
 * @param val	[in] Number.
 */
void GcnFstTest::print_uint32_en_US(ostream &os, uint32_t val)
{
	char buf[16];
	char *p = &buf[0];
	unsigned int digits = 0;
	do {
		if (digits > 0 && (digits % 3 == 0)) {
			// en_US thousands separator.
			*p++ = ',';
		}
		assert(p < &buf[sizeof(buf)]);
		*p++ = '0' + (val % 10);
		val /= 10;
		digits++;
	} while (val != 0);
	assert(p < &buf[sizeof(buf)]);
	*p = 0;

	// Number is formatted, but it's in reverse-order.
	while (p > &buf[0]) {
		os << *--p;
	}
}

/**
 * Make sure there aren't any duplicate filenames in all subdirectories.
 */
TEST_P(GcnFstTest, NoDuplicateFilenames)
{
	ASSERT_NO_FATAL_FAILURE(checkNoDuplicateFilenames("/"));
	EXPECT_FALSE(m_fst->hasErrors());
}

/**
 * Print the FST directory structure and compare it to a known-good version.
 */
TEST_P(GcnFstTest, FstPrint)
{
	// Parameterized test.
	const GcnFstTest_mode &mode = GetParam();

	// Open the Zip file.
	// NOTE: MiniZip 2.2.3's compatibility functions
	// take UTF-8 on Windows, not UTF-16.
	const char *zip_filename = nullptr;
	switch (mode.offsetShift) {
		case 0:
			zip_filename = "GameCube.fst.txt.zip";
			break;
		case 2:
			zip_filename = "Wii.fst.txt.zip";
			break;
		default:
			ASSERT_TRUE(false) << "offsetShift is " << (int)mode.offsetShift << "; should be either 0 or 2.";
	}

	// Replace ".bin" in the FST filename with ".txt".
	string fst_txt_filename = mode.fst_filename;
	ASSERT_GT(fst_txt_filename.size(), 8U) <<
		"Internal filename '" << mode.fst_filename << "' doesn't have a '.fst.bin' extension.";
	ASSERT_EQ(".fst.bin", fst_txt_filename.substr(fst_txt_filename.size() - 8)) <<
		"Internal filename '" << mode.fst_filename << "' doesn't have a '.fst.bin' extension.";
	fst_txt_filename.replace(fst_txt_filename.size() - 8, 8, ".fst.txt");

	// Get the known-good FST printout.
	ao::uvector<uint8_t> fst_txt_buf;
	ASSERT_GT(getFileFromZip(zip_filename, fst_txt_filename.c_str(), fst_txt_buf, MAX_GCN_FST_TXT_FILESIZE), 0);

	// Import the FST text into an istringstream.
	istringstream fst_text_expected(
		string(reinterpret_cast<const char*>(fst_txt_buf.data()), fst_txt_buf.size()));

	// Print the FST.bin to a new stringstream.
	FstFileCount fc = {0, 0};
	stringstream fst_text_actual;
	fstPrint(m_fst, fst_text_actual, &fc);

	// Print the directory/file count manually in order to
	// avoid locale issues. (Test data uses en_US.)
	fst_text_actual << '\n';
	print_uint32_en_US(fst_text_actual, fc.dirs);
	fst_text_actual << ' ' << (fc.dirs == 1 ? "directory" : "directories") << ", ";
	print_uint32_en_US(fst_text_actual, fc.files);
	fst_text_actual << ' ' << (fc.files == 1 ? "file" : "files") << '\n';

	// Compare the two stringstreams.
	// NOTE: Only Unix line endings are supported.
	char line_buf_actual[1024], line_buf_expected[1024];
	for (int line_num = 1; /* condition is defined in the loop */; line_num++) {
		bool ok_actual = (bool)fst_text_actual.getline(line_buf_actual, sizeof(line_buf_actual), '\n');
		bool ok_expected = (bool)fst_text_expected.getline(line_buf_expected, sizeof(line_buf_expected), '\n');
		if (!ok_actual && !ok_expected) {
			// End of both files.
			break;
		}

		EXPECT_TRUE(ok_actual) << "Unexpected EOF in FST text generated by fstPrint().";
		EXPECT_TRUE(ok_expected) << "Unexpected EOF in FST text from '" << fst_txt_filename << "'.";
		if (!ok_actual || !ok_expected) {
			// End of one of the files.
			break;
		}

		// Compare the two strings.
		// TODO: Eliminate one of the comparisons?
		EXPECT_STREQ(line_buf_expected, line_buf_actual) <<
			"Line " << line_num << " differs between fstPrint() and '" << fst_txt_filename << "'.";
		if (strcmp(line_buf_expected, line_buf_actual) != 0) {
			// Lines don't match.
			break;
		}
	};
	EXPECT_FALSE(m_fst->hasErrors());
}

/** Test case parameters. **/

/**
 * Get the list of FST files from a Zip file.
 * @param offsetShift File offset shift. (0 == GCN, 2 == Wii)
 * @return FST files.
 */
std::vector<GcnFstTest_mode> GcnFstTest::ReadTestCasesFromDisk(uint8_t offsetShift)
{
	// NOTE: Cannot use ASSERT_TRUE() here.
	std::vector<GcnFstTest_mode> files;

	// Open the Zip file.
	// NOTE: MiniZip 2.2.3's compatibility functions
	// take UTF-8 on Windows, not UTF-16.
	const char *zip_filename;
	switch (offsetShift) {
		case 0:
			zip_filename = "GameCube.fst.bin.zip";
			break;
		case 2:
			zip_filename = "Wii.fst.bin.zip";
			break;
		default:
			EXPECT_TRUE(false) << "offsetShift is " << (int)offsetShift << "; should be either 0 or 2.";
			return files;
	}

	unzFile unz = openZip(zip_filename);
	EXPECT_TRUE(unz != nullptr) <<
		"Could not open '" << zip_filename << "', check the test directory!";
	if (!unz) {
		return files;
	}

	// MiniZip 2.x (up to 2.2.3) doesn't automatically go to the first file.
	// Hence, we'll need to do that here.
	int ret = unzGoToFirstFile(unz);
	EXPECT_EQ(0, ret) << "unzGoToFirstFile failed in '" << zip_filename << "'.";
	if (ret != 0) {
		unzClose(unz);
		return files;
	}

	// Read the filenames.
	char filename[256];
	unz_file_info64 file_info;
	do {
		ret = unzGetCurrentFileInfo64(unz,
			&file_info,
			filename, sizeof(filename),
			nullptr, 0,
			nullptr, 0);
		if (ret != UNZ_OK)
			break;

		// Make sure the filename isn't empty.
		EXPECT_GT(file_info.size_filename, 0) << "A filename in the ZIP file has no name. Skipping...";

		// Make sure the file isn't too big.
		EXPECT_LE(file_info.uncompressed_size, (uLong)MAX_GCN_FST_BIN_FILESIZE) <<
			"GCN FST file '" << filename << "' is too big. (maximum size is 1 MB)";

		if (file_info.size_filename > 0 &&
		    file_info.uncompressed_size <= (uLong)MAX_GCN_FST_BIN_FILESIZE)
		{
			// Add this filename to the list.
			// NOTE: Filename might not be NULL-terminated,
			// so use the explicit length.
			GcnFstTest_mode mode(string(filename, file_info.size_filename), offsetShift);
			files.push_back(mode);
		}

		// Next file.
		ret = unzGoToNextFile(unz);
	} while (ret == UNZ_OK);
	unzClose(unz);

	// Handle "end of file list" as OK.
	if (ret == UNZ_END_OF_LIST_OF_FILE)
		ret = UNZ_OK;

	EXPECT_EQ(UNZ_OK, ret);
	EXPECT_GT(files.size(), 0U);
	return files;
}

/**
 * Test case suffix generator.
 * @param info Test parameter information.
 * @return Test case suffix.
 */
string GcnFstTest::test_case_suffix_generator(const ::testing::TestParamInfo<GcnFstTest_mode> &info)
{
	string suffix = info.param.fst_filename;

	// Replace all non-alphanumeric characters with '_'.
	// See gtest-param-util.h::IsValidParamName().
	for (int i = (int)suffix.size()-1; i >= 0; i--) {
		char chr = suffix[i];
		if (!isalnum(chr) && chr != '_') {
			suffix[i] = '_';
		}
	}

	return suffix;
}

INSTANTIATE_TEST_CASE_P(GameCube, GcnFstTest,
	testing::ValuesIn(GcnFstTest::ReadTestCasesFromDisk(0))
	, GcnFstTest::test_case_suffix_generator);

INSTANTIATE_TEST_CASE_P(Wii, GcnFstTest,
	testing::ValuesIn(GcnFstTest::ReadTestCasesFromDisk(2))
	, GcnFstTest::test_case_suffix_generator);

} }

extern "C" int gtest_main(int argc, char *argv[])
{
	fprintf(stderr, "LibRomData test suite: GcnFst tests.\n\n");
	fflush(nullptr);

	// Make sure the CRC32 table is initialized.
	get_crc_table();

	// coverity[fun_call_w_exception]: uncaught exceptions cause nonzero exit anyway, so don't warn.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
