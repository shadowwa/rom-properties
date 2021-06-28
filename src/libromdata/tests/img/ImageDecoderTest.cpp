/***************************************************************************
 * ROM Properties Page shell extension. (libromdata/tests)                 *
 * ImageDecoderTest.cpp: ImageDecoder class test.                          *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "config.librpbase.h"
#include "config.libromdata.h"
#include "config.librptexture.h"

// Google Test
#include "gtest/gtest.h"
#include "tcharx.h"

// zlib and libpng
#include <zlib.h>
#ifdef HAVE_PNG
# include <png.h>
#endif /* HAVE_PNG */

// gzclose_r() and gzclose_w() were introduced in zlib-1.2.4.
#if (ZLIB_VER_MAJOR > 1) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR > 2) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR == 2 && ZLIB_VER_REVISION >= 4)
// zlib-1.2.4 or later
#else
#define gzclose_r(file) gzclose(file)
#define gzclose_w(file) gzclose(file)
#endif

// librpbase, librpfile
#include "common.h"
#include "librpbase/img/RpImageLoader.hpp"
#include "librpfile/RpFile.hpp"
#include "librpfile/RpMemFile.hpp"
#include "librpfile/FileSystem.hpp"
using namespace LibRpBase;
using namespace LibRpFile;

// librptexture
#include "librptexture/img/rp_image.hpp"
#include "librptexture/decoder/ImageDecoder.hpp"
using namespace LibRpTexture;

// TODO: Separate out the actual DDS texture loader
// from the RomData subclass?
#include "Other/RpTextureWrapper.hpp"

// ROM images. Used for console-specific image formats.
#include "Console/DreamcastSave.hpp"
#include "Console/GameCubeSave.hpp"
#include "Console/PlayStationSave.hpp"
#include "Handheld/NintendoDS.hpp"
#include "Handheld/Nintendo3DS_SMDH.hpp"
#include "Other/NintendoBadge.hpp"

// C includes.
#include <stdint.h>
#include <stdlib.h>

// C includes. (C++ namespace)
#include "ctypex.h"
#include <cstring>

// C++ includes.
#include <functional>
#include <memory>
#include <string>
using std::string;
using std::unique_ptr;

// Uninitialized vector class.
// Reference: http://andreoffringa.org/?q=uvector
#include "uvector.h"

namespace LibRomData { namespace Tests {

struct ImageDecoderTest_mode
{
	string dds_gz_filename;
	string png_filename;
	RomData::ImageType imgType;

	ImageDecoderTest_mode(
		const char *dds_gz_filename,
		const char *png_filename,
		RomData::ImageType imgType = RomData::IMG_INT_IMAGE
		)
		: dds_gz_filename(dds_gz_filename)
		, png_filename(png_filename)
		, imgType(imgType)
	{ }

	// May be required for MSVC 2010?
	ImageDecoderTest_mode(const ImageDecoderTest_mode &other)
		: dds_gz_filename(other.dds_gz_filename)
		, png_filename(other.png_filename)
		, imgType(other.imgType)
	{ }

	// Required for MSVC 2010.
	ImageDecoderTest_mode &operator=(const ImageDecoderTest_mode &other)
	{
		dds_gz_filename = other.dds_gz_filename;
		png_filename = other.png_filename;
		imgType = other.imgType;
		return *this;
	}
};

// Maximum file size for images.
static const size_t MAX_DDS_IMAGE_FILESIZE = 12*1024*1024;
static const size_t MAX_PNG_IMAGE_FILESIZE =  2*1024*1024;

class ImageDecoderTest : public ::testing::TestWithParam<ImageDecoderTest_mode>
{
	protected:
		ImageDecoderTest()
			: ::testing::TestWithParam<ImageDecoderTest_mode>()
			, m_gzDds(nullptr)
			, m_f_dds(nullptr)
			, m_romData(nullptr)
		{ }

		void SetUp(void) final;
		void TearDown(void) final;

	public:
		/**
		 * Compare two rp_image objects.
		 * If either rp_image is CI8, a copy of the image
		 * will be created in ARGB32 for comparison purposes.
		 * @param pImgExpected	[in] Expected image data.
		 * @param pImgActual	[in] Actual image data.
		 */
		static void Compare_RpImage(
			const rp_image *pImgExpected,
			const rp_image *pImgActual);

		// Number of iterations for benchmarks.
		static const unsigned int BENCHMARK_ITERATIONS = 1000;
		static const unsigned int BENCHMARK_ITERATIONS_BC7 = 100;

	public:
		// Image buffers.
		ao::uvector<uint8_t> m_dds_buf;
		ao::uvector<uint8_t> m_png_buf;

		// gzip file handle for .dds.gz.
		// Placed here so it can be freed by TearDown() if necessary.
		gzFile m_gzDds;

		// RomData class pointer for .dds.gz.
		// Placed here so it can be freed by TearDown() if necessary.
		// The underlying RpMemFile is here as well, since we can't
		// delete it before deleting the RomData object.
		RpMemFile *m_f_dds;
		RomData *m_romData;

	public:
		/** Test case parameters. **/

		/**
		 * Test case suffix generator.
		 * @param info Test parameter information.
		 * @return Test case suffix.
		 */
		static string test_case_suffix_generator(const ::testing::TestParamInfo<ImageDecoderTest_mode> &info);

		/**
		 * Replace slashes with backslashes on Windows.
		 * @param path Pathname.
		 */
		static inline void replace_slashes(string &path);

		/**
		 * Internal test function.
		 */
		void decodeTest_internal(void);

		/**
		 * Internal benchmark function.
		 */
		void decodeBenchmark_internal(void);
};

/**
 * Formatting function for ImageDecoderTest.
 */
inline ::std::ostream& operator<<(::std::ostream& os, const ImageDecoderTest_mode& mode)
{
	return os << mode.dds_gz_filename;
};

/**
 * Replace slashes with backslashes on Windows.
 * @param path Pathname.
 */
inline void ImageDecoderTest::replace_slashes(string &path)
{
#ifdef _WIN32
	std::for_each(path.begin(), path.end(), [](char &p) {
		if (p == '/') {
			p = '\\';
		}
	});
#else
	// Nothing to do here...
	RP_UNUSED(path);
#endif /* _WIN32 */
}

/**
 * SetUp() function.
 * Run before each test.
 */
void ImageDecoderTest::SetUp(void)
{
	if (::testing::UnitTest::GetInstance()->current_test_info()->value_param() == nullptr) {
		// Not a parameterized test.
		return;
	}

	// Parameterized test.
	const ImageDecoderTest_mode &mode = GetParam();

	// Open the gzipped DDS texture file being tested.
	string path = "ImageDecoder_data";
	path += DIR_SEP_CHR;
	path += mode.dds_gz_filename;
	replace_slashes(path);
	m_gzDds = gzopen(path.c_str(), "rb");
	ASSERT_TRUE(m_gzDds != nullptr) << "gzopen() failed to open the DDS file: "
		<< mode.dds_gz_filename;

	// Get the decompressed file size.
	// gzseek() does not support SEEK_END.
	// Read through the file until we hit an EOF.
	// NOTE: We could optimize this by reading the uncompressed
	// file size if gzdirect() == 1, but this is a test case,
	// so it doesn't really matter.
	uint8_t buf[4096];
	uint32_t ddsSize = 0;
	while (!gzeof(m_gzDds)) {
		int sz_read = gzread(m_gzDds, buf, sizeof(buf));
		ASSERT_NE(sz_read, -1) << "gzread() failed.";
		ddsSize += sz_read;
	}
	gzrewind(m_gzDds);

	/* FIXME: Per-type minimum sizes.
	 * This fails on some very small SVR files.
	ASSERT_GT(ddsSize, 4+sizeof(DDS_HEADER))
		<< "DDS test image is too small.";
	*/
	ASSERT_LE(ddsSize, MAX_DDS_IMAGE_FILESIZE)
		<< "DDS test image is too big.";

	// Read the DDS image into memory.
	m_dds_buf.resize(ddsSize);
	ASSERT_EQ((size_t)ddsSize, m_dds_buf.size());
	int sz = gzread(m_gzDds, m_dds_buf.data(), ddsSize);
	gzclose_r(m_gzDds);
	m_gzDds = nullptr;

	ASSERT_EQ(ddsSize, (uint32_t)sz) << "Error loading DDS image file: " <<
		mode.dds_gz_filename << " - short read";

	// Open the PNG image file being tested.
	path.resize(18);	// Back to "ImageDecoder_data/".
	path += mode.png_filename;
	replace_slashes(path);
	unique_RefBase<RpFile> file(new RpFile(path, RpFile::FM_OPEN_READ));
	ASSERT_TRUE(file->isOpen()) << "Error loading PNG image file: " <<
		mode.png_filename << " - " << strerror(file->lastError());

	// Maximum image size.
	ASSERT_LE(file->size(), MAX_PNG_IMAGE_FILESIZE) << "PNG test image is too big.";

	// Read the PNG image into memory.
	const size_t pngSize = static_cast<size_t>(file->size());
	m_png_buf.resize(pngSize);
	ASSERT_EQ(pngSize, m_png_buf.size());
	size_t readSize = file->read(m_png_buf.data(), pngSize);
	ASSERT_EQ(pngSize, readSize) << "Error loading PNG image file: " <<
		mode.png_filename << " - short read";
}

/**
 * TearDown() function.
 * Run after each test.
 */
void ImageDecoderTest::TearDown(void)
{
	UNREF_AND_NULL(m_romData);
	UNREF_AND_NULL(m_f_dds);

	if (m_gzDds) {
		gzclose_r(m_gzDds);
		m_gzDds = nullptr;
	}
}

struct RpImageUnrefDeleter {
	void operator()(rp_image *img) {
		UNREF(img);
	}
};

/**
 * Compare two rp_image objects.
 * If either rp_image is CI8, a copy of the image
 * will be created in ARGB32 for comparison purposes.
 * @param pImgExpected	[in] Expected image data.
 * @param pImgActual	[in] Actual image data.
 */
void ImageDecoderTest::Compare_RpImage(
	const rp_image *pImgExpected,
	const rp_image *pImgActual)
{
	// Make sure we have two ARGB32 images with equal sizes.
	ASSERT_TRUE(pImgExpected->isValid()) << "pImgExpected is not valid.";
	ASSERT_TRUE(pImgActual->isValid())   << "pImgActual is not valid.";
	ASSERT_EQ(pImgExpected->width(),  pImgActual->width())  << "Image sizes don't match.";
	ASSERT_EQ(pImgExpected->height(), pImgActual->height()) << "Image sizes don't match.";

	// Ensure we delete temporary images if they're created.
	unique_ptr<rp_image, RpImageUnrefDeleter> tmpImg_expected(nullptr, RpImageUnrefDeleter());
	unique_ptr<rp_image, RpImageUnrefDeleter> tmpImg_actual(nullptr, RpImageUnrefDeleter());

	switch (pImgExpected->format()) {
		case rp_image::Format::ARGB32:
			// No conversion needed.
			break;

		case rp_image::Format::CI8:
			// Convert to ARGB32.
			tmpImg_expected.reset(pImgExpected->dup_ARGB32());
			ASSERT_TRUE(tmpImg_expected != nullptr);
			ASSERT_TRUE(tmpImg_expected->isValid());
			pImgExpected = tmpImg_expected.get();
			break;

		default:
			ASSERT_TRUE(false) << "pImgExpected: Invalid pixel format for this test.";
			break;
	}

	switch (pImgActual->format()) {
		case rp_image::Format::ARGB32:
			// No conversion needed.
			break;

		case rp_image::Format::CI8:
			// Convert to ARGB32.
			tmpImg_actual.reset(pImgActual->dup_ARGB32());
			ASSERT_TRUE(tmpImg_actual != nullptr);
			ASSERT_TRUE(tmpImg_actual->isValid());
			pImgActual = tmpImg_actual.get();
			break;

		default:
			ASSERT_TRUE(false) << "pImgActual: Invalid pixel format for this test.";
			break;
	}

	// Compare the two images.
	// TODO: rp_image::operator==()?
	const uint32_t *pBitsExpected = static_cast<const uint32_t*>(pImgExpected->bits());
	const uint32_t *pBitsActual   = static_cast<const uint32_t*>(pImgActual->bits());
	const int stride_diff_exp = (pImgExpected->stride() - pImgExpected->row_bytes()) / sizeof(uint32_t);
	const int stride_diff_act = (pImgActual->stride() - pImgActual->row_bytes()) / sizeof(uint32_t);
	const unsigned int width  = static_cast<unsigned int>(pImgExpected->width());
	const unsigned int height = static_cast<unsigned int>(pImgExpected->height());
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++, pBitsExpected++, pBitsActual++) {
			if (*pBitsExpected != *pBitsActual) {
				printf("ERR: (%u,%u): expected %08X, got %08X\n",
					x, y, *pBitsExpected, *pBitsActual);
			}
			ASSERT_EQ(*pBitsExpected, *pBitsActual) <<
				"Decoded image does not match the expected PNG image.";
		}
		pBitsExpected += stride_diff_exp;
		pBitsActual   += stride_diff_act;
	}
}

/**
 * Internal test function.
 */
void ImageDecoderTest::decodeTest_internal(void)
{
	// Parameterized test.
	const ImageDecoderTest_mode &mode = GetParam();

	// Load the PNG image.
	unique_RefBase<RpMemFile> f_png(new RpMemFile(m_png_buf.data(), m_png_buf.size()));
	ASSERT_TRUE(f_png->isOpen()) << "Could not create RpMemFile for the PNG image.";
	unique_ptr<rp_image, RpImageUnrefDeleter> img_png(RpImageLoader::load(f_png.get()), RpImageUnrefDeleter());
	ASSERT_TRUE(img_png != nullptr) << "Could not load the PNG image as rp_image.";
	ASSERT_TRUE(img_png->isValid()) << "Could not load the PNG image as rp_image.";

	// Open the image as an IRpFile.
	m_f_dds = new RpMemFile(m_dds_buf.data(), m_dds_buf.size());
	ASSERT_TRUE(m_f_dds->isOpen()) << "Could not create RpMemFile for the DDS image.";

	// Determine the image type by checking the last 7 characters of the filename.
	const char *filetype = nullptr;
	ASSERT_GT(mode.dds_gz_filename.size(), 7U);
	if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".dds.gz") ||
	    !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-4, 4, ".dds")) {
		// DDS image
		// NOTE: Using RpTextureWrapper.
		filetype = "DDS";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".pvr.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".gvr.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".svr.gz")) {
		// PVR/GVR/SVR image
		// NOTE: Using RpTextureWrapper.
		// NOTE: May be PowerVR3.
		filetype = "PVR";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".ktx.gz")) {
		// Khronos KTX image
		// TODO: Use .zktx format instead of .ktx.gz.
		// NOTE: Using RpTextureWrapper.
		filetype = "KTX";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-8, 8, ".ktx2.gz")) {
		// Khronos KTX2 image
		// TODO: Use .zktx2 format instead of .ktx2.gz.
		// NOTE: Using RpTextureWrapper.
		filetype = "KTX2";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-11, 11, ".ps3.vtf.gz")) {
		// Valve Texture File (PS3)
		// NOTE: Using RpTextureWrapper.
		filetype = "VTF3";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".vtf.gz")) {
		// Valve Texture File
		// NOTE: Using RpTextureWrapper.
		filetype = "VTF";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (mode.dds_gz_filename.size() >= 8U &&
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-8, 8, ".smdh.gz"))
	{
		// Nintendo 3DS SMDH file
		filetype = "SMDH";
		m_romData = new Nintendo3DS_SMDH(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".gci.gz")) {
		// Nintendo GameCube save file
		filetype = "GCI";
		m_romData = new GameCubeSave(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".VMS.gz")) {
		// Sega Dreamcast save file
		filetype = "VMS";
		m_romData = new DreamcastSave(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".PSV.gz")) {
		// Sony PlayStation save file
		filetype = "PSV";
		m_romData = new PlayStationSave(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".nds.gz")) {
		// Nintendo DS ROM image
		filetype = "NDS";
		m_romData = new NintendoDS(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".cab.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".prb.gz")) {
		// Nintendo Badge Arcade texture
		filetype = "NintendoBadge";
		m_romData = new NintendoBadge(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-4, 4, ".tex")) {
		// Leapster Didj texture
		// NOTE: Using RpTextureWrapper.
		filetype = "DidjTex";
		m_romData = new RpTextureWrapper(m_f_dds);
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".tga.gz")) {
		// TrueVision TGA
		// NOTE: Using RpTextureWrapper.
		// NOTE 2: TGA detection is currently done by file extension,
		// so we need to set the RpMemFile's filename.
		filetype = "TGA";
		m_f_dds->setFilename(mode.dds_gz_filename);
		m_romData = new RpTextureWrapper(m_f_dds);
	} else {
		ASSERT_TRUE(false) << "Unknown image type.";
	}
	ASSERT_TRUE(m_romData->isValid()) << "Could not load the " << filetype << " image.";
	ASSERT_TRUE(m_romData->isOpen()) << "Could not load the " << filetype << " image.";

	// Get the DDS image as an rp_image.
	const rp_image *const img_dds = m_romData->image(mode.imgType);
	ASSERT_TRUE(img_dds != nullptr) << "Could not load the " << filetype << " image as rp_image.";

	// Get the image again.
	// The pointer should be identical to the first one.
	const rp_image *const img_dds_2 = m_romData->image(mode.imgType);
	EXPECT_EQ(img_dds, img_dds_2) << "Retrieving the image twice resulted in a different rp_image object.";

	// Compare the image data.
	ASSERT_NO_FATAL_FAILURE(Compare_RpImage(img_png.get(), img_dds));
}

/**
 * Run an ImageDecoder test.
 */
TEST_P(ImageDecoderTest, decodeTest)
{
	ASSERT_NO_FATAL_FAILURE(decodeTest_internal());
}

/**
 * Internal benchmark function.
 */
void ImageDecoderTest::decodeBenchmark_internal(void)
{
	// Parameterized test.
	const ImageDecoderTest_mode &mode = GetParam();

	// Open the image as an IRpFile.
	m_f_dds = new RpMemFile(m_dds_buf.data(), m_dds_buf.size());
	ASSERT_TRUE(m_f_dds->isOpen()) << "Could not create RpMemFile for the DDS image.";

	// NOTE: We can't simply decode the image multiple times.
	// We have to reopen the RomData subclass every time.

	// Benchmark iterations.
	// BC7 has fewer iterations because it's more complicated.
	unsigned int max_iterations;
	if (mode.dds_gz_filename.find("BC7/") == 0) {
		// This is BC7.
		max_iterations = BENCHMARK_ITERATIONS_BC7;
	} else {
		// Not BC7.
		max_iterations = BENCHMARK_ITERATIONS;
	}

	// Constructor function.
	std::function<RomData*(IRpFile*)> fn_ctor;

	// Determine the image type by checking the last 7 characters of the filename.
	ASSERT_GT(mode.dds_gz_filename.size(), 7U);
	if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".dds.gz") ||
	    !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-4, 4, ".dds")) {
		// DDS image
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".pvr.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".gvr.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".svr.gz")) {
		// PVR/GVR/SVR image
		// NOTE: Using RpTextureWrapper.
		// NOTE: May be PowerVR3.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".ktx.gz")) {
		// Khronos KTX image
		// TODO: Use .zktx format instead of .ktx.gz?
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-8, 8, ".ktx2.gz")) {
		// Khronos KTX image
		// TODO: Use .zktx2 format instead of .ktx2.gz?
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-11, 11, ".ps3.vtf.gz")) {
		// Valve Texture File (PS3)
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".vtf.gz")) {
		// Valve Texture File
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (mode.dds_gz_filename.size() >= 8U &&
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-8, 8, ".smdh.gz"))
	{
		// Nintendo 3DS SMDH file
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new Nintendo3DS_SMDH(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".gci.gz")) {
		// Nintendo GameCube save file
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new GameCubeSave(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".VMS.gz")) {
		// Sega Dreamcast save file
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new DreamcastSave(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".PSV.gz")) {
		// Sony PlayStation save file
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new PlayStationSave(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".nds.gz")) {
		// Nintendo DS ROM image
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new NintendoDS(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".cab.gz") ||
		   !mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".prb.gz")) {
		// Nintendo Badge Arcade texture
		// NOTE: Increased iterations due to smaller files.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new NintendoBadge(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-4, 4, ".tex")) {
		// Leapster Didj texture
		// NOTE: Increased iterations due to smaller files.
		// NOTE: Using RpTextureWrapper.
		max_iterations *= 10;
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else if (!mode.dds_gz_filename.compare(mode.dds_gz_filename.size()-7, 7, ".tga.gz")) {
		// TrueVision TGA
		// NOTE: Using RpTextureWrapper.
		fn_ctor = [](IRpFile *file) { return new RpTextureWrapper(file); };
	} else {
		ASSERT_TRUE(false) << "Unknown image type.";
	}

	ASSERT_TRUE(fn_ctor) << "Unable to get a constructor function.";

	for (unsigned int i = max_iterations; i > 0; i--) {
		m_romData = fn_ctor(m_f_dds);
		ASSERT_TRUE(m_romData->isValid()) << "Could not load the source image.";
		ASSERT_TRUE(m_romData->isOpen()) << "Could not load the source image.";

		// Get the source image as an rp_image.
		// TODO: imgType to string?
		const rp_image *const img_dds = m_romData->image(mode.imgType);
		ASSERT_TRUE(img_dds != nullptr) << "Could not load the source image as rp_image.";

		UNREF_AND_NULL_NOCHK(m_romData);
	}
}

/**
 * Benchmark an ImageDecoder test.
 */
TEST_P(ImageDecoderTest, decodeBenchmark)
{
	ASSERT_NO_FATAL_FAILURE(decodeBenchmark_internal());
}

/**
 * Test case suffix generator.
 * @param info Test parameter information.
 * @return Test case suffix.
 */
string ImageDecoderTest::test_case_suffix_generator(const ::testing::TestParamInfo<ImageDecoderTest_mode> &info)
{
	string suffix = info.param.dds_gz_filename;

	// Replace all non-alphanumeric characters with '_'.
	// See gtest-param-util.h::IsValidParamName().
	std::for_each(suffix.begin(), suffix.end(),
		[](char &c) {
			// NOTE: Not checking for '_' because that
			// wastes a branch.
			if (!ISALNUM(c)) {
				c = '_';
			}
		}
	);

	// Append the image type to allow checking multiple types
	// of images in the same file.
	static const char s_imgType[][8] = {
		"_Icon", "_Banner", "_Media", "_Image"
	};
	static_assert(ARRAY_SIZE(s_imgType) == RomData::IMG_INT_MAX - RomData::IMG_INT_MIN + 1,
		"s_imgType[] needs to be updated.");
	assert(info.param.imgType >= RomData::IMG_INT_MIN);
	assert(info.param.imgType <= RomData::IMG_INT_MAX);
	if (info.param.imgType >= RomData::IMG_INT_MIN && info.param.imgType <= RomData::IMG_INT_MAX) {
		suffix += s_imgType[info.param.imgType - RomData::IMG_INT_MIN];
	}

	// TODO: Convert to ASCII?
	return suffix;
}

// Test cases.

// DirectDrawSurface tests. (S3TC)
INSTANTIATE_TEST_SUITE_P(DDS_S3TC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"S3TC/dxt1-rgb.dds.gz",
			"S3TC/dxt1-rgb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt2-rgb.dds.gz",
			"S3TC/dxt2-rgb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt2-argb.dds.gz",
			"S3TC/dxt2-argb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt3-rgb.dds.gz",
			"S3TC/dxt3-rgb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt3-argb.dds.gz",
			"S3TC/dxt3-argb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt4-rgb.dds.gz",
			"S3TC/dxt4-rgb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt4-argb.dds.gz",
			"S3TC/dxt4-argb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt5-rgb.dds.gz",
			"S3TC/dxt5-rgb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/dxt5-argb.dds.gz",
			"S3TC/dxt5-argb.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/bc4.dds.gz",
			"S3TC/bc4.s3tc.png"),
		ImageDecoderTest_mode(
			"S3TC/bc5.dds.gz",
			"S3TC/bc5.s3tc.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 16-bit RGB)
INSTANTIATE_TEST_SUITE_P(DDS_RGB16, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"RGB/RGB565.dds.gz",
			"RGB/RGB565.png"),
		ImageDecoderTest_mode(
			"RGB/xRGB4444.dds.gz",
			"RGB/xRGB4444.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 16-bit ARGB)
INSTANTIATE_TEST_SUITE_P(DDS_ARGB16, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"ARGB/ARGB1555.dds.gz",
			"ARGB/ARGB1555.png"),
		ImageDecoderTest_mode(
			"ARGB/ARGB4444.dds.gz",
			"ARGB/ARGB4444.png"),
		ImageDecoderTest_mode(
			"ARGB/ARGB8332.dds.gz",
			"ARGB/ARGB8332.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 15-bit RGB)
INSTANTIATE_TEST_SUITE_P(DDS_RGB15, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"RGB/RGB565.dds.gz",
			"RGB/RGB565.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 24-bit RGB)
INSTANTIATE_TEST_SUITE_P(DDS_RGB24, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"RGB/RGB888.dds.gz",
			"RGB/RGB888.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 32-bit RGB)
INSTANTIATE_TEST_SUITE_P(DDS_RGB32, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"RGB/xRGB8888.dds.gz",
			"RGB/xRGB8888.png"),
		ImageDecoderTest_mode(
			"RGB/xBGR8888.dds.gz",
			"RGB/xBGR8888.png"),

		// Uncommon formats.
		ImageDecoderTest_mode(
			"RGB/G16R16.dds.gz",
			"RGB/G16R16.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Uncompressed 32-bit ARGB)
INSTANTIATE_TEST_SUITE_P(DDS_ARGB32, ImageDecoderTest,
	::testing::Values(
		// 32-bit
		ImageDecoderTest_mode(
			"ARGB/ARGB8888.dds.gz",
			"ARGB/ARGB8888.png"),
		ImageDecoderTest_mode(
			"ARGB/ABGR8888.dds.gz",
			"ARGB/ABGR8888.png"),

		// Uncommon formats.
		ImageDecoderTest_mode(
			"ARGB/A2R10G10B10.dds.gz",
			"ARGB/A2R10G10B10.png"),
		ImageDecoderTest_mode(
			"ARGB/A2B10G10R10.dds.gz",
			"ARGB/A2B10G10R10.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Luminance)
INSTANTIATE_TEST_SUITE_P(DDS_Luma, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"Luma/L8.dds.gz",
			"Luma/L8.png"),
		ImageDecoderTest_mode(
			"Luma/A4L4.dds.gz",
			"Luma/A4L4.png"),
		ImageDecoderTest_mode(
			"Luma/L16.dds.gz",
			"Luma/L16.png"),
		ImageDecoderTest_mode(
			"Luma/A8L8.dds.gz",
			"Luma/A8L8.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// DirectDrawSurface tests. (Alpha)
INSTANTIATE_TEST_SUITE_P(DDS_Alpha, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"Alpha/A8.dds.gz",
			"Alpha/A8.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// PVR tests. (square twiddled)
INSTANTIATE_TEST_SUITE_P(PVR_SqTwiddled, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"PVR/bg_00.pvr.gz",
			"PVR/bg_00.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// PVR tests. (VQ)
INSTANTIATE_TEST_SUITE_P(PVR_VQ, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"PVR/mr_128k_huti.pvr.gz",
			"PVR/mr_128k_huti.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// PVR tests. (Small VQ)
INSTANTIATE_TEST_SUITE_P(PVR_SmallVQ, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"PVR/drumfuta1.pvr.gz",
			"PVR/drumfuta1.png"),
		ImageDecoderTest_mode(
			"PVR/drum_ref.pvr.gz",
			"PVR/drum_ref.png"),
		ImageDecoderTest_mode(
			"PVR/sp_blue.pvr.gz",
			"PVR/sp_blue.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// GVR tests. (RGB5A3)
INSTANTIATE_TEST_SUITE_P(GVR_RGB5A3, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"GVR/zanki_sonic.gvr.gz",
			"GVR/zanki_sonic.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// GVR tests. (DXT1, S3TC)
INSTANTIATE_TEST_SUITE_P(GVR_DXT1_S3TC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"GVR/paldam_off.gvr.gz",
			"GVR/paldam_off.s3tc.png"),
		ImageDecoderTest_mode(
			"GVR/paldam_on.gvr.gz",
			"GVR/paldam_on.s3tc.png"),
		ImageDecoderTest_mode(
			"GVR/weeklytitle.gvr.gz",
			"GVR/weeklytitle.s3tc.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// KTX tests.
#define KTX_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"KTX/" file ".ktx.gz", \
			"KTX/" file ".png")
INSTANTIATE_TEST_SUITE_P(KTX, ImageDecoderTest,
	::testing::Values(
		// RGB reference image.
		ImageDecoderTest_mode(
			"KTX/rgb-reference.ktx.gz",
			"KTX/rgb.png"),
		// RGB reference image, mipmap levels == 0
		ImageDecoderTest_mode(
			"KTX/rgb-amg-reference.ktx.gz",
			"KTX/rgb.png"),
		// Orientation: Up (upside-down compared to "normal")
		ImageDecoderTest_mode(
			"KTX/up-reference.ktx.gz",
			"KTX/up.png"),
		// Orientation: Down (same as "normal")
		ImageDecoderTest_mode(
			"KTX/down-reference.ktx.gz",
			"KTX/up.png"),

		// Luminance (unsized: GL_LUMINANCE)
		ImageDecoderTest_mode(
			"KTX/luminance_unsized_reference.ktx.gz",
			"KTX/luminance.png"),
		// Luminance (sized: GL_LUMINANCE8)
		ImageDecoderTest_mode(
			"KTX/luminance_sized_reference.ktx.gz",
			"KTX/luminance.png"),

		// ETC1
		KTX_IMAGE_TEST("etc1"),
		// ETC2
		KTX_IMAGE_TEST("etc2-rgb"),
		KTX_IMAGE_TEST("etc2-rgba1"),
		KTX_IMAGE_TEST("etc2-rgba8"),

		// BGR888 (Hi Corp)
		KTX_IMAGE_TEST("hi_mark"),
		KTX_IMAGE_TEST("hi_mark_sq"),

		// RGBA reference image.
		ImageDecoderTest_mode(
			"KTX/rgba-reference.ktx.gz",
			"KTX/rgba.png"))

	, ImageDecoderTest::test_case_suffix_generator);

// KTX2 tests.
#define KTX2_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"KTX2/" file ".ktx2.gz", \
			"KTX2/" file ".png")
INSTANTIATE_TEST_SUITE_P(KTX2, ImageDecoderTest,
	::testing::Values(
		KTX2_IMAGE_TEST("cubemap_yokohama_bc3_unorm"),
		KTX2_IMAGE_TEST("cubemap_yokohama_etc2_unorm"),
		KTX2_IMAGE_TEST("orient-down-metadata-u"),
		KTX2_IMAGE_TEST("orient-up-metadata-u"),
		KTX2_IMAGE_TEST("pattern_02_bc2"),
		KTX2_IMAGE_TEST("rgba-reference-u"),
		KTX2_IMAGE_TEST("rgb-mipmap-reference-u"),
		KTX2_IMAGE_TEST("texturearray_bc3_unorm"),
		KTX2_IMAGE_TEST("texturearray_etc2_unorm"))
	, ImageDecoderTest::test_case_suffix_generator);

// Valve VTF tests. (all formats)
INSTANTIATE_TEST_SUITE_P(VTF, ImageDecoderTest,
	::testing::Values(
		// NOTE: VTF channel ordering is usually backwards from ImageDecoder.

		// 32-bit ARGB
		ImageDecoderTest_mode(
			"VTF/ABGR8888.vtf.gz",
			"argb-reference.png"),
		ImageDecoderTest_mode(
			"VTF/ARGB8888.vtf.gz",	// NOTE: Actually RABG8888.
			"argb-reference.png"),
		ImageDecoderTest_mode(
			"VTF/BGRA8888.vtf.gz",
			"argb-reference.png"),
		ImageDecoderTest_mode(
			"VTF/RGBA8888.vtf.gz",
			"argb-reference.png"),

		// 32-bit xRGB
		ImageDecoderTest_mode(
			"VTF/BGRx8888.vtf.gz",
			"rgb-reference.png"),

		// 24-bit RGB
		ImageDecoderTest_mode(
			"VTF/BGR888.vtf.gz",
			"rgb-reference.png"),
		ImageDecoderTest_mode(
			"VTF/RGB888.vtf.gz",
			"rgb-reference.png"),

		// 24-bit RGB + bluescreen
		ImageDecoderTest_mode(
			"VTF/BGR888_bluescreen.vtf.gz",
			"VTF/BGR888_bluescreen.png"),
		ImageDecoderTest_mode(
			"VTF/RGB888_bluescreen.vtf.gz",
			"VTF/BGR888_bluescreen.png"),

		// 16-bit RGB (565)
		// FIXME: Tests are failing.
		ImageDecoderTest_mode(
			"VTF/BGR565.vtf.gz",
			"RGB/RGB565.png"),
		ImageDecoderTest_mode(
			"VTF/RGB565.vtf.gz",
			"RGB/RGB565.png"),

		// 15-bit RGB (555)
		ImageDecoderTest_mode(
			"VTF/BGRx5551.vtf.gz",
			"RGB/RGB555.png"),

		// 16-bit ARGB (4444)
		ImageDecoderTest_mode(
			"VTF/BGRA4444.vtf.gz",
			"ARGB/ARGB4444.png"),

		// UV88 (handled as RG88)
		ImageDecoderTest_mode(
			"VTF/UV88.vtf.gz",
			"rg-reference.png"),

		// Intensity formats
		ImageDecoderTest_mode(
			"VTF/I8.vtf.gz",
			"Luma/L8.png"),
		ImageDecoderTest_mode(
			"VTF/IA88.vtf.gz",
			"Luma/A8L8.png"),

		// Alpha format (A8)
		ImageDecoderTest_mode(
			"VTF/A8.vtf.gz",
			"Alpha/A8.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// Valve VTF tests. (S3TC)
INSTANTIATE_TEST_SUITE_P(VTF_S3TC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"VTF/DXT1.vtf.gz",
			"VTF/DXT1.s3tc.png"),
		ImageDecoderTest_mode(
			"VTF/DXT1_A1.vtf.gz",
			"VTF/DXT1_A1.s3tc.png"),
		ImageDecoderTest_mode(
			"VTF/DXT3.vtf.gz",
			"VTF/DXT3.s3tc.png"),
		ImageDecoderTest_mode(
			"VTF/DXT5.vtf.gz",
			"VTF/DXT5.s3tc.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// Valve VTF3 tests. (S3TC)
INSTANTIATE_TEST_SUITE_P(VTF3_S3TC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"VTF3/elevator_screen_broken_normal.ps3.vtf.gz",
			"VTF3/elevator_screen_broken_normal.ps3.s3tc.png"),
		ImageDecoderTest_mode(
			"VTF3/elevator_screen_colour.ps3.vtf.gz",
			"VTF3/elevator_screen_colour.ps3.s3tc.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// Test images from texture-compressor.
// Reference: https://github.com/TimvanScherpenzeel/texture-compressor
INSTANTIATE_TEST_SUITE_P(TCtest, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"tctest/example-etc1.ktx.gz",
			"tctest/example-etc1.ktx.png"),
		ImageDecoderTest_mode(
			"tctest/example-etc2.ktx.gz",
			"tctest/example-etc2.ktx.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// texture-compressor tests. (S3TC)
INSTANTIATE_TEST_SUITE_P(TCtest_S3TC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"tctest/example-dxt1.dds.gz",
			"tctest/example-dxt1.s3tc.dds.png"),
		ImageDecoderTest_mode(
			"tctest/example-dxt3.dds.gz",
			"tctest/example-dxt5.s3tc.dds.png"),
		ImageDecoderTest_mode(
			"tctest/example-dxt5.dds.gz",
			"tctest/example-dxt5.s3tc.dds.png"))
	, ImageDecoderTest::test_case_suffix_generator);

#ifdef ENABLE_PVRTC
// texture-compressor tests. (PVRTC)
INSTANTIATE_TEST_SUITE_P(TCtest_PVRTC, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"tctest/example-pvrtc1.pvr.gz",
			"tctest/example-pvrtc1.pvr.png"),
		ImageDecoderTest_mode(
			"tctest/example-pvrtc2-2bpp.pvr.gz",
			"tctest/example-pvrtc2-2bpp.pvr.png"),
		ImageDecoderTest_mode(
			"tctest/example-pvrtc2-4bpp.pvr.gz",
			"tctest/example-pvrtc2-4bpp.pvr.png"))
	, ImageDecoderTest::test_case_suffix_generator);
#endif /* ENABLE_PVRTC */

// BC7 tests.
INSTANTIATE_TEST_SUITE_P(BC7, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"BC7/w5_grass200_abd_a.dds.gz",
			"BC7/w5_grass200_abd_a.png"),
		ImageDecoderTest_mode(
			"BC7/w5_grass201_abd.dds.gz",
			"BC7/w5_grass201_abd.png"),
		ImageDecoderTest_mode(
			"BC7/w5_grass206_abd.dds.gz",
			"BC7/w5_grass206_abd.png"),
		ImageDecoderTest_mode(
			"BC7/w5_rock805_abd.dds.gz",
			"BC7/w5_rock805_abd.png"),
		ImageDecoderTest_mode(
			"BC7/w5_rock805_nrm.dds.gz",
			"BC7/w5_rock805_nrm.png"),
		ImageDecoderTest_mode(
			"BC7/w5_rope801_prm.dds",
			"BC7/w5_rope801_prm.png"),
		ImageDecoderTest_mode(
			"BC7/w5_sand504_abd_a.dds.gz",
			"BC7/w5_sand504_abd_a.png"),
		ImageDecoderTest_mode(
			"BC7/w5_wood503_prm.dds.gz",
			"BC7/w5_wood503_prm.png"))
	, ImageDecoderTest::test_case_suffix_generator);

// SMDH tests.
// From *New* Nintendo 3DS 9.2.0-20J.
#define SMDH_TEST(file) ImageDecoderTest_mode( \
			"SMDH/" file ".smdh.gz", \
			"SMDH/" file ".png", \
			RomData::IMG_INT_ICON)
INSTANTIATE_TEST_SUITE_P(SMDH, ImageDecoderTest,
	::testing::Values(
		SMDH_TEST("0004001000020000"),
		SMDH_TEST("0004001000020100"),
		SMDH_TEST("0004001000020400"),
		SMDH_TEST("0004001000020900"),
		SMDH_TEST("0004001000020A00"),
		SMDH_TEST("000400100002BF00"),
		SMDH_TEST("0004001020020300"),
		SMDH_TEST("0004001020020D00"),
		SMDH_TEST("0004001020023100"),
		SMDH_TEST("000400102002C800"),
		SMDH_TEST("000400102002C900"),
		SMDH_TEST("000400102002CB00"),
		SMDH_TEST("0004003000008302"),
		SMDH_TEST("0004003000008402"),
		SMDH_TEST("0004003000008602"),
		SMDH_TEST("0004003000008702"),
		SMDH_TEST("0004003000008D02"),
		SMDH_TEST("0004003000008E02"),
		SMDH_TEST("000400300000BC02"),
		SMDH_TEST("000400300000C602"),
		SMDH_TEST("0004003020008802"))
	, ImageDecoderTest::test_case_suffix_generator);

// GCI tests.
// TODO: Use something like GcnFstTest that uses an array of filenames
// to generate tests at runtime instead of compile-time?
#define GCI_ICON_TEST(file) ImageDecoderTest_mode( \
			"GCI/" file ".gci.gz", \
			"GCI/" file ".icon.png", \
			RomData::IMG_INT_ICON)
#define GCI_BANNER_TEST(file) ImageDecoderTest_mode( \
			"GCI/" file ".gci.gz", \
			"GCI/" file ".banner.png", \
			RomData::IMG_INT_BANNER)

INSTANTIATE_TEST_SUITE_P(GCI_Icon_1, ImageDecoderTest,
	::testing::Values(
		GCI_ICON_TEST("01-D43E-ZELDA"),
		GCI_ICON_TEST("01-G2ME-MetroidPrime2"),
		GCI_ICON_TEST("01-G4SE-gc4sword"),
		GCI_ICON_TEST("01-G8ME-mariost_save_file"),
		GCI_ICON_TEST("01-GAFE-DobutsunomoriP_MURA"),
		GCI_ICON_TEST("01-GALE-SuperSmashBros0110290334"),
		GCI_ICON_TEST("01-GC6E-pokemon_colosseum"),
		GCI_ICON_TEST("01-GEDE-Eternal Darkness"),
		GCI_ICON_TEST("01-GFEE-FIREEMBLEM8J"),
		GCI_ICON_TEST("01-GKGE-DONKEY KONGA SAVEDATA"),
		GCI_ICON_TEST("01-GLME-LUIGI_MANSION_SAVEDATA_v3"),
		GCI_ICON_TEST("01-GM4E-MarioKart Double Dash!!"),
		GCI_ICON_TEST("01-GM8E-MetroidPrime A"),
		GCI_ICON_TEST("01-GMPE-MARIPA4BOX0"),
		GCI_ICON_TEST("01-GMPE-MARIPA4BOX1"),
		GCI_ICON_TEST("01-GMPE-MARIPA4BOX2"),
		GCI_ICON_TEST("01-GMSE-super_mario_sunshine"),
		GCI_ICON_TEST("01-GP5E-MARIPA5"),
		GCI_ICON_TEST("01-GP6E-MARIPA6"),
		GCI_ICON_TEST("01-GP7E-MARIPA7"),
		GCI_ICON_TEST("01-GPIE-Pikmin dataFile"),
		GCI_ICON_TEST("01-GPVE-Pikmin2_SaveData"),
		GCI_ICON_TEST("01-GPXE-pokemon_rs_memory_box"),
		GCI_ICON_TEST("01-GXXE-PokemonXD"),
		GCI_ICON_TEST("01-GYBE-MainData"),
		GCI_ICON_TEST("01-GZ2E-gczelda2"),
		GCI_ICON_TEST("01-GZLE-gczelda"),
		GCI_ICON_TEST("01-PZLE-NES_ZELDA1_SAVE"),
		GCI_ICON_TEST("01-PZLE-NES_ZELDA2_SAVE"),
		GCI_ICON_TEST("01-PZLE-ZELDA1"),
		GCI_ICON_TEST("01-PZLE-ZELDA2"),
		GCI_ICON_TEST("51-GTKE-Save Game0"),
		GCI_ICON_TEST("52-GTDE-SK5sbltitgaSK5sbltitga"),
		GCI_ICON_TEST("52-GTDE-SK5sirpvsicSK5sirpvsic"),
		GCI_ICON_TEST("52-GTDE-SK5xwkqsbafSK5xwkqsbaf"),
		GCI_ICON_TEST("5D-GE9E-EDEDDNEDDYTHEMIS-EDVENTURES"),
		GCI_ICON_TEST("69-GHSE-POTTERCOS"),
		GCI_ICON_TEST("69-GO7E-BOND"),
		GCI_ICON_TEST("78-GW3E-__2f__w_mania2002"),
		GCI_ICON_TEST("7D-GCBE-CRASHWOC"),
		GCI_ICON_TEST("7D-GCNE-all"),
		GCI_ICON_TEST("8P-G2XE-SONIC GEMS_00"),
		GCI_ICON_TEST("8P-G2XE-SONIC_R"),
		GCI_ICON_TEST("8P-G2XE-STF.DAT"),
		GCI_ICON_TEST("8P-G9SE-SONICHEROES_00"),
		GCI_ICON_TEST("8P-G9SE-SONICHEROES_01"),
		GCI_ICON_TEST("8P-G9SE-SONICHEROES_02"),
		GCI_ICON_TEST("8P-GEZE-billyhatcher"),
		GCI_ICON_TEST("8P-GFZE-f_zero.dat"))
	, ImageDecoderTest::test_case_suffix_generator);

INSTANTIATE_TEST_SUITE_P(GCI_Icon_2, ImageDecoderTest,
	::testing::Values(
		GCI_ICON_TEST("8P-GM2E-rep0000010000C900002497A48E.dat"),
		GCI_ICON_TEST("8P-GM2E-super_monkey_ball_2.dat"),
		GCI_ICON_TEST("8P-GMBE-smkb0058556041f42afb"),
		GCI_ICON_TEST("8P-GMBE-super_monkey_ball.sys"),
		GCI_ICON_TEST("8P-GPUE-Puyo Pop Fever Replay01"),
		GCI_ICON_TEST("8P-GPUE-Puyo Pop Fever System"),
		GCI_ICON_TEST("8P-GSNE-SONIC2B__5f____5f__S01"),
		GCI_ICON_TEST("8P-GSOE-S_MEGA_SYS"),
		GCI_ICON_TEST("8P-GUPE-SHADOWTHEHEDGEHOG"),
		GCI_ICON_TEST("8P-GXEE-SONICRIDERS_GAMEDATA_01"),
		GCI_ICON_TEST("8P-GXSE-SONICADVENTURE_DX_PLAYRECORD_1"),
		GCI_ICON_TEST("AF-GNME-NAMCOMUSEUM"),
		GCI_ICON_TEST("AF-GP2E-PMW2SAVE"),
		GCI_ICON_TEST("AF-GPME-PACMANFEVER"))
	, ImageDecoderTest::test_case_suffix_generator);

// NOTE: Some files don't have banners. They're left in the list for
// consistency, but are commented out.
INSTANTIATE_TEST_SUITE_P(GCI_Banner_1, ImageDecoderTest,
	::testing::Values(
		GCI_BANNER_TEST("01-D43E-ZELDA"),
		GCI_BANNER_TEST("01-G2ME-MetroidPrime2"),
		GCI_BANNER_TEST("01-G4SE-gc4sword"),
		GCI_BANNER_TEST("01-G8ME-mariost_save_file"),
		GCI_BANNER_TEST("01-GAFE-DobutsunomoriP_MURA"),
		GCI_BANNER_TEST("01-GALE-SuperSmashBros0110290334"),
		GCI_BANNER_TEST("01-GC6E-pokemon_colosseum"),
		GCI_BANNER_TEST("01-GEDE-Eternal Darkness"),
		GCI_BANNER_TEST("01-GFEE-FIREEMBLEM8J"),
		GCI_BANNER_TEST("01-GKGE-DONKEY KONGA SAVEDATA"),
		GCI_BANNER_TEST("01-GLME-LUIGI_MANSION_SAVEDATA_v3"),
		GCI_BANNER_TEST("01-GM4E-MarioKart Double Dash!!"),
		GCI_BANNER_TEST("01-GM8E-MetroidPrime A"),
		GCI_BANNER_TEST("01-GMPE-MARIPA4BOX0"),
		GCI_BANNER_TEST("01-GMPE-MARIPA4BOX1"),
		GCI_BANNER_TEST("01-GMPE-MARIPA4BOX2"),
		GCI_BANNER_TEST("01-GMSE-super_mario_sunshine"),
		GCI_BANNER_TEST("01-GP5E-MARIPA5"),
		GCI_BANNER_TEST("01-GP6E-MARIPA6"),
		GCI_BANNER_TEST("01-GP7E-MARIPA7"),
		GCI_BANNER_TEST("01-GPIE-Pikmin dataFile"),
		GCI_BANNER_TEST("01-GPVE-Pikmin2_SaveData"),
		GCI_BANNER_TEST("01-GPXE-pokemon_rs_memory_box"),
		GCI_BANNER_TEST("01-GXXE-PokemonXD"),
		GCI_BANNER_TEST("01-GYBE-MainData"),
		GCI_BANNER_TEST("01-GZ2E-gczelda2"),
		GCI_BANNER_TEST("01-GZLE-gczelda"),
		GCI_BANNER_TEST("01-PZLE-NES_ZELDA1_SAVE"),
		GCI_BANNER_TEST("01-PZLE-NES_ZELDA2_SAVE"),
		GCI_BANNER_TEST("01-PZLE-ZELDA1"),
		GCI_BANNER_TEST("01-PZLE-ZELDA2"),
		//GCI_BANNER_TEST("51-GTKE-Save Game0"),
		GCI_BANNER_TEST("52-GTDE-SK5sbltitgaSK5sbltitga"),
		GCI_BANNER_TEST("52-GTDE-SK5sirpvsicSK5sirpvsic"),
		GCI_BANNER_TEST("52-GTDE-SK5xwkqsbafSK5xwkqsbaf"),
		//GCI_BANNER_TEST("5D-GE9E-EDEDDNEDDYTHEMIS-EDVENTURES"),
		//GCI_BANNER_TEST("69-GHSE-POTTERCOS"),
		GCI_BANNER_TEST("69-GO7E-BOND"),
		GCI_BANNER_TEST("78-GW3E-__2f__w_mania2002"),
		//GCI_BANNER_TEST("7D-GCBE-CRASHWOC"),
		GCI_BANNER_TEST("7D-GCNE-all"),
		GCI_BANNER_TEST("8P-G2XE-SONIC GEMS_00"),
		GCI_BANNER_TEST("8P-G2XE-SONIC_R"),
		GCI_BANNER_TEST("8P-G2XE-STF.DAT"),
		GCI_BANNER_TEST("8P-G9SE-SONICHEROES_00"),
		GCI_BANNER_TEST("8P-G9SE-SONICHEROES_01"),
		GCI_BANNER_TEST("8P-G9SE-SONICHEROES_02"),
		GCI_BANNER_TEST("8P-GEZE-billyhatcher"),
		GCI_BANNER_TEST("8P-GFZE-f_zero.dat"))
	, ImageDecoderTest::test_case_suffix_generator);

INSTANTIATE_TEST_SUITE_P(GCI_Banner_2, ImageDecoderTest,
	::testing::Values(
		GCI_BANNER_TEST("8P-GM2E-rep0000010000C900002497A48E.dat"),
		GCI_BANNER_TEST("8P-GM2E-super_monkey_ball_2.dat"),
		GCI_BANNER_TEST("8P-GMBE-smkb0058556041f42afb"),
		GCI_BANNER_TEST("8P-GMBE-super_monkey_ball.sys"),
		//GCI_BANNER_TEST("8P-GPUE-Puyo Pop Fever Replay01"),
		//GCI_BANNER_TEST("8P-GPUE-Puyo Pop Fever System"),
		GCI_BANNER_TEST("8P-GSNE-SONIC2B__5f____5f__S01"),
		GCI_BANNER_TEST("8P-GSOE-S_MEGA_SYS"),
		GCI_BANNER_TEST("8P-GUPE-SHADOWTHEHEDGEHOG"),
		GCI_BANNER_TEST("8P-GXEE-SONICRIDERS_GAMEDATA_01"),
		GCI_BANNER_TEST("8P-GXSE-SONICADVENTURE_DX_PLAYRECORD_1"),
		GCI_BANNER_TEST("AF-GNME-NAMCOMUSEUM"),
		GCI_BANNER_TEST("AF-GP2E-PMW2SAVE"),
		GCI_BANNER_TEST("AF-GPME-PACMANFEVER"))
	, ImageDecoderTest::test_case_suffix_generator);

// VMS tests.
INSTANTIATE_TEST_SUITE_P(VMS, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"Misc/BIOS002.VMS.gz",
			"Misc/BIOS002.png",
			RomData::IMG_INT_ICON),
		ImageDecoderTest_mode(
			"Misc/SONIC2C.VMS.gz",
			"Misc/SONIC2C.png",
			RomData::IMG_INT_ICON))
	, ImageDecoderTest::test_case_suffix_generator);

// PSV tests.
INSTANTIATE_TEST_SUITE_P(PSV, ImageDecoderTest,
	::testing::Values(
		ImageDecoderTest_mode(
			"Misc/BASCUS-94228535059524F.PSV.gz",
			"Misc/BASCUS-94228535059524F.png",
			RomData::IMG_INT_ICON),
		ImageDecoderTest_mode(
			"Misc/BASCUS-949003034323235383937.PSV.gz",
			"Misc/BASCUS-949003034323235383937.png",
			RomData::IMG_INT_ICON))
	, ImageDecoderTest::test_case_suffix_generator);

// NDS tests.
// TODO: Use something like GcnFstTest that uses an array of filenames
// to generate tests at runtime instead of compile-time?
#define NDS_ICON_TEST(file) ImageDecoderTest_mode( \
			"NDS/" file ".header-icon.nds.gz", \
			"NDS/" file ".header-icon.png", \
			RomData::IMG_INT_ICON)

INSTANTIATE_TEST_SUITE_P(NDS, ImageDecoderTest,
	::testing::Values(
		NDS_ICON_TEST("A2DE01"),
		NDS_ICON_TEST("A3YE8P"),
		NDS_ICON_TEST("AIZE01"),
		NDS_ICON_TEST("AKWE01"),
		NDS_ICON_TEST("AMHE01"),
		NDS_ICON_TEST("ANDE01"),
		NDS_ICON_TEST("ANME01"),
		NDS_ICON_TEST("AOSE01"),
		NDS_ICON_TEST("APAE01"),
		NDS_ICON_TEST("ASCE8P"),
		NDS_ICON_TEST("ASME01"),
		NDS_ICON_TEST("ATKE01"),
		NDS_ICON_TEST("AY9E8P"),
		NDS_ICON_TEST("AYWE01"),
		NDS_ICON_TEST("BFUE41"),
		NDS_ICON_TEST("BOOE08"),
		NDS_ICON_TEST("BSLEWR"),
		NDS_ICON_TEST("BXSE8P"),
		NDS_ICON_TEST("CBQEG9"),
		NDS_ICON_TEST("COLE8P"),
		NDS_ICON_TEST("CS3E8P"),
		NDS_ICON_TEST("DMFEA4"),
		NDS_ICON_TEST("DSYESZ"),
		NDS_ICON_TEST("NTRJ01.Tetris-THQ"),
		NDS_ICON_TEST("VSOE8P"),
		NDS_ICON_TEST("YDLE20"),
		NDS_ICON_TEST("YLZE01"),
		NDS_ICON_TEST("YWSE8P"))
	, ImageDecoderTest::test_case_suffix_generator);

// NintendoBadge tests.
// TODO: Use something like GcnFstTest that uses an array of filenames
// to generate tests at runtime instead of compile-time?
#define BADGE_IMAGE_ONLY_TEST(file) ImageDecoderTest_mode( \
			"Misc/" file ".gz", \
			"Misc/" file ".image.png", \
			RomData::IMG_INT_IMAGE)
#define BADGE_ICON_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"Misc/" file ".gz", \
			"Misc/" file ".icon.png", \
			RomData::IMG_INT_ICON), \
			BADGE_IMAGE_ONLY_TEST(file)
INSTANTIATE_TEST_SUITE_P(NintendoBadge, ImageDecoderTest,
	::testing::Values(
		BADGE_ICON_IMAGE_TEST("MroKrt8.cab"),
		BADGE_IMAGE_ONLY_TEST("MroKrt8_Chara_Luigi000.prb"),
		BADGE_IMAGE_ONLY_TEST("MroKrt8_Chara_Mario000.prb"),
		BADGE_IMAGE_ONLY_TEST("MroKrt8_Chara_Peach000.prb"),
		BADGE_IMAGE_ONLY_TEST("Pr_Animal_12Sc_edit.prb"),
		BADGE_ICON_IMAGE_TEST("Pr_Animal_17Sc_mset.prb"),
		BADGE_ICON_IMAGE_TEST("Pr_FcRemix_2_drM_item05.prb"),
		BADGE_ICON_IMAGE_TEST("Pr_FcRemix_2_punch_char01_3_Sep.prb"))
	, ImageDecoderTest::test_case_suffix_generator);

// SVR tests.
// TODO: Use something like GcnFstTest that uses an array of filenames
// to generate tests at runtime instead of compile-time?
// FIXME: Puyo Tools doesn't support format 0x61.
#define SVR_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"SVR/" file ".svr.gz", \
			"SVR/" file ".png")
INSTANTIATE_TEST_SUITE_P(SVR_1, ImageDecoderTest,
	::testing::Values(
		SVR_IMAGE_TEST("1channel_01"),
		SVR_IMAGE_TEST("1channel_02"),
		SVR_IMAGE_TEST("1channel_03"),
		SVR_IMAGE_TEST("1channel_04"),
		SVR_IMAGE_TEST("1channel_05"),
		SVR_IMAGE_TEST("1channel_06"),
		SVR_IMAGE_TEST("2channel_01"),
		SVR_IMAGE_TEST("2channel_02"),
		SVR_IMAGE_TEST("2channel_03"),
		SVR_IMAGE_TEST("3channel_01"),
		SVR_IMAGE_TEST("3channel_02"),
		SVR_IMAGE_TEST("3channel_03"),
		SVR_IMAGE_TEST("al_egg01"),
		SVR_IMAGE_TEST("ani_han_karada"),
		SVR_IMAGE_TEST("ani_han_parts"),
		SVR_IMAGE_TEST("ani_kao_005"),
		SVR_IMAGE_TEST("c_butterfly_k00"),
		SVR_IMAGE_TEST("c_env_k00"),
		//SVR_IMAGE_TEST("c_env_k01"),
		SVR_IMAGE_TEST("c_env_k02"),
		SVR_IMAGE_TEST("c_env_k04"),
		//SVR_IMAGE_TEST("c_gold_k00"),
		//SVR_IMAGE_TEST("c_green_k00"),
		SVR_IMAGE_TEST("c_green_k01"),
		SVR_IMAGE_TEST("channel_off"),
		SVR_IMAGE_TEST("cha_trt_back01"),
		//SVR_IMAGE_TEST("cha_trt_shadow01"),
		SVR_IMAGE_TEST("cha_trt_water01"),
		SVR_IMAGE_TEST("cha_trt_water02"),
		SVR_IMAGE_TEST("c_paint_aiai"),
		SVR_IMAGE_TEST("c_paint_akira"),
		SVR_IMAGE_TEST("c_paint_amigo"),
		SVR_IMAGE_TEST("c_paint_cake"),
		SVR_IMAGE_TEST("c_paint_chao"),
		SVR_IMAGE_TEST("c_paint_chu"),
		SVR_IMAGE_TEST("c_paint_egg"),
		SVR_IMAGE_TEST("c_paint_flower"),
		SVR_IMAGE_TEST("c_paint_nights"),
		SVR_IMAGE_TEST("c_paint_puyo"),
		SVR_IMAGE_TEST("c_paint_shadow"),
		SVR_IMAGE_TEST("c_paint_soccer"),
		SVR_IMAGE_TEST("c_paint_sonic"),
		SVR_IMAGE_TEST("c_paint_taxi"),
		SVR_IMAGE_TEST("c_paint_urara"),
		SVR_IMAGE_TEST("c_paint_zombie"),
		SVR_IMAGE_TEST("c_pumpkinhead"),
		SVR_IMAGE_TEST("cs_book_02"),
		//SVR_IMAGE_TEST("c_silver_k00"),
		SVR_IMAGE_TEST("c_toy_k00"))
		//SVR_IMAGE_TEST("c_toy_k01"))
	, ImageDecoderTest::test_case_suffix_generator);

INSTANTIATE_TEST_SUITE_P(SVR_2, ImageDecoderTest,
	::testing::Values(
		SVR_IMAGE_TEST("c_toy_k02"),
		SVR_IMAGE_TEST("c_toy_k03"),
		SVR_IMAGE_TEST("c_toy_k04"),
		//SVR_IMAGE_TEST("c_tree_k00"),
		//SVR_IMAGE_TEST("c_tree_k01"),
		//SVR_IMAGE_TEST("eft_drop_t01"),
		//SVR_IMAGE_TEST("eft_ripple01"),
		//SVR_IMAGE_TEST("eft_splash_t01"),
		SVR_IMAGE_TEST("kin_blbaketu2"),
		SVR_IMAGE_TEST("kin_blbaketu"),
		SVR_IMAGE_TEST("kin_blcarpet"),
		SVR_IMAGE_TEST("kin_bldai"),
		SVR_IMAGE_TEST("kin_bldesk1"),
		SVR_IMAGE_TEST("kin_blfire"),
		SVR_IMAGE_TEST("kin_blhako1"),
		SVR_IMAGE_TEST("kin_blhako2"),
		SVR_IMAGE_TEST("kin_blhako3"),
		SVR_IMAGE_TEST("kin_blhako4"),
		SVR_IMAGE_TEST("kin_blhako5"),
		SVR_IMAGE_TEST("kin_blhako6"),
		SVR_IMAGE_TEST("kin_blhouki2"),
		SVR_IMAGE_TEST("kin_blhouki3"),
		SVR_IMAGE_TEST("kin_blhouki4"),
		SVR_IMAGE_TEST("kin_blhouki5"),
		SVR_IMAGE_TEST("kin_blhouki"),
		SVR_IMAGE_TEST("kin_blkabekake1"),
		SVR_IMAGE_TEST("kin_blkabekake2"),
		SVR_IMAGE_TEST("kin_blkasa"),
		SVR_IMAGE_TEST("kin_blkumo2"),
		SVR_IMAGE_TEST("kin_blkumo"),
		SVR_IMAGE_TEST("kin_bllamp1"),
		SVR_IMAGE_TEST("kin_bllamp2"),
		SVR_IMAGE_TEST("kin_bllamp3"),
		SVR_IMAGE_TEST("kin_blmaf"),
		SVR_IMAGE_TEST("kin_blmask"),
		SVR_IMAGE_TEST("kin_blphot"),
		SVR_IMAGE_TEST("kin_blregi2"),
		SVR_IMAGE_TEST("kin_blregi"),
		SVR_IMAGE_TEST("kin_blreji3"),
		SVR_IMAGE_TEST("kin_blreji4"),
		SVR_IMAGE_TEST("kin_blreji5"),
		SVR_IMAGE_TEST("kin_blspeeker2"),
		SVR_IMAGE_TEST("kin_blspeeker3"),
		SVR_IMAGE_TEST("kin_blspeeker"),
		SVR_IMAGE_TEST("kin_blvase1"),
		SVR_IMAGE_TEST("kin_blvase2"),
		SVR_IMAGE_TEST("kin_blwall2"),
		SVR_IMAGE_TEST("kin_blwall"),
		SVR_IMAGE_TEST("kin_cbdoor2"),
		SVR_IMAGE_TEST("kin_copdwaku2"))
	, ImageDecoderTest::test_case_suffix_generator);

INSTANTIATE_TEST_SUITE_P(SVR_3, ImageDecoderTest,
	::testing::Values(
		SVR_IMAGE_TEST("kin_doglass1"),
		SVR_IMAGE_TEST("kin_dolabel2"),
		SVR_IMAGE_TEST("kin_dolabel3"),
		SVR_IMAGE_TEST("kin_kagebkaku"),
		SVR_IMAGE_TEST("kin_kagebmaru"),
		SVR_IMAGE_TEST("kin_kobookstand"),
		SVR_IMAGE_TEST("kin_koedbook2"),
		SVR_IMAGE_TEST("kin_kohibook2"),
		SVR_IMAGE_TEST("kin_kohibook3"),
		SVR_IMAGE_TEST("kin_kokami"),
		SVR_IMAGE_TEST("kin_kophotstand1"),
		SVR_IMAGE_TEST("kin_koribbon"),
		SVR_IMAGE_TEST("trt_g00_gass"),
		SVR_IMAGE_TEST("trt_g00_lawn00small"),
		SVR_IMAGE_TEST("trt_g00_river01"),
		SVR_IMAGE_TEST("trt_g00_river02"))
	, ImageDecoderTest::test_case_suffix_generator);

// NOTE: DidjTex files aren't gzipped because the texture data is
// internally compressed using zlib.
#define DidjTex_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"DidjTex/" file ".tex", \
			"DidjTex/" file ".png")
INSTANTIATE_TEST_SUITE_P(DidjTex, ImageDecoderTest,
	::testing::Values(
		DidjTex_IMAGE_TEST("LeftArrow"),
		DidjTex_IMAGE_TEST("LightOff"),
		DidjTex_IMAGE_TEST("Slider"),
		DidjTex_IMAGE_TEST("StaticTVImage"),
		DidjTex_IMAGE_TEST("Zone1Act1Icon_Alpha"),
		DidjTex_IMAGE_TEST("Zone1Act1Icon"))
	, ImageDecoderTest::test_case_suffix_generator);

#ifdef ENABLE_PVRTC
// PowerVR3 tests.
#define PowerVR3_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"PowerVR3/" file ".pvr.gz", \
			"PowerVR3/" file ".pvr.png")
INSTANTIATE_TEST_SUITE_P(PowerVR3, ImageDecoderTest,
	::testing::Values(
		//PowerVR3_IMAGE_TEST("brdfLUT"),				// TODO: R16fG16f
		//PowerVR3_IMAGE_TEST("GnomeHorde-bigMushroom_texture"),	// FIXME: Failing (PVRTC-I 4bpp RGB)
		//PowerVR3_IMAGE_TEST("GnomeHorde-fern"),			// FIXME: Failing (PVRTC-I 4bpp RGBA)
		PowerVR3_IMAGE_TEST("Navigation3D-font"),
		//PowerVR3_IMAGE_TEST("Navigation3D-Road"),			// FIXME: Failing (LA88)
		//PowerVR3_IMAGE_TEST("Satyr-Table"),				// FIXME: Failing (RGBA8888)
		PowerVR3_IMAGE_TEST("text-fri")					// 32x16, caused rp_image::flip(FLIP_V) to break
		)
	, ImageDecoderTest::test_case_suffix_generator);
#endif /* ENABLE_PVRTC */

// TGA tests.
#define TGA_IMAGE_TEST(file) ImageDecoderTest_mode( \
			"TGA/" file ".tga.gz", \
			"TGA/" file ".png")
INSTANTIATE_TEST_SUITE_P(TGA, ImageDecoderTest,
	::testing::Values(
		// Reference images.
		ImageDecoderTest_mode("TGA/TGA_1_CM24_IM8.tga.gz", "CI8-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_1_CM32_IM8.tga.gz", "CI8a-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_2_24.tga.gz", "rgb-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_2_32.tga.gz", "argb-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_3_8.tga.gz", "gray-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_9_CM24_IM8.tga.gz", "CI8-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_9_CM32_IM8.tga.gz", "CI8a-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_10_24.tga.gz", "rgb-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_10_32.tga.gz", "argb-reference.png"),
		ImageDecoderTest_mode("TGA/TGA_11_8.tga.gz", "gray-reference.png"),

		// TGA 2.0 conformance test suite
		// FIXME: utc16, utc32 have incorrect alpha values?
		// Both gimp and imagemagick interpret them as completely transparent.
		TGA_IMAGE_TEST("conformance/cbw8"),
		TGA_IMAGE_TEST("conformance/ccm8"),
		TGA_IMAGE_TEST("conformance/ctc24"),
		TGA_IMAGE_TEST("conformance/ubw8"),
		TGA_IMAGE_TEST("conformance/ucm8"),
		//TGA_IMAGE_TEST("conformance/utc16"),
		TGA_IMAGE_TEST("conformance/utc24"),
		//TGA_IMAGE_TEST("conformance/utc32"),

		// Test images from tga-go
		// https://github.com/ftrvxmtrx/tga
		// FIXME: Some incorrect alpha values...
		// NOTE: The rgb24/rgb32 colormap images use .1.png; others use .0.png.
		//ImageDecoderTest_mode("TGA/tga-go/ctc16.tga.gz", "TGA/tga-go/color.png"),
		ImageDecoderTest_mode("TGA/tga-go/monochrome8_bottom_left_rle.tga.gz", "TGA/tga-go/monochrome8.png"),
		ImageDecoderTest_mode("TGA/tga-go/monochrome8_bottom_left.tga.gz", "TGA/tga-go/monochrome8.png"),
		//ImageDecoderTest_mode("TGA/tga-go/monochrome16_bottom_left_rle.tga.gz", "TGA/tga-go/monochrome16.png"),
		//ImageDecoderTest_mode("TGA/tga-go/monochrome16_bottom_left.tga.gz", "TGA/tga-go/monochrome16.png"),
		ImageDecoderTest_mode("TGA/tga-go/rgb24_bottom_left_rle.tga.gz", "TGA/tga-go/rgb24.0.png"),
		ImageDecoderTest_mode("TGA/tga-go/rgb24_top_left_colormap.tga.gz", "TGA/tga-go/rgb24.1.png"),
		ImageDecoderTest_mode("TGA/tga-go/rgb24_top_left.tga.gz", "TGA/tga-go/rgb24.0.png"),
		ImageDecoderTest_mode("TGA/tga-go/rgb32_bottom_left.tga.gz", "TGA/tga-go/rgb32.0.png"),
		//ImageDecoderTest_mode("TGA/tga-go/rgb32_top_left_rle_colormap.tga.gz", "TGA/tga-go/rgb32.1.png"),
		ImageDecoderTest_mode("TGA/tga-go/rgb32_top_left_rle.tga.gz", "TGA/tga-go/rgb32.0.png"))
	, ImageDecoderTest::test_case_suffix_generator);

} }

/**
 * Test suite main function.
 * Called by gtest_init.cpp.
 */
extern "C" int gtest_main(int argc, TCHAR *argv[])
{
	fprintf(stderr, "LibRomData test suite: ImageDecoder tests.\n\n");
	fprintf(stderr, "Benchmark iterations: %u (%u for BC7)\n",
		LibRomData::Tests::ImageDecoderTest::BENCHMARK_ITERATIONS,
		LibRomData::Tests::ImageDecoderTest::BENCHMARK_ITERATIONS_BC7);
	fflush(nullptr);

	// coverity[fun_call_w_exception]: uncaught exceptions cause nonzero exit anyway, so don't warn.
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
