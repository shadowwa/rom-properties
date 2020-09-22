/****************************************************************************
 * ROM Properties Page shell extension. (librptexture)                      *
 * didj_tex_structs.h: Leapster Didj .tex format data structures.           *
 *                                                                          *
 * Copyright (c) 2019-2020 by David Korth.                                  *
 * SPDX-License-Identifier: GPL-2.0-or-later                                *
 ****************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DIDJ_TEX_STRUCTS_H__
#define __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DIDJ_TEX_STRUCTS_H__

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Leapster Didj .tex: File header.
 * Reverse-engineered from Didj .tex files.
 *
 * NOTE: The actual image size is usually a power of two.
 * It should be rescaled to the display size when rendering.
 * rom-properties will use the actual size.
 *
 * All fields are in little-endian.
 */
#define DIDJ_TEX_HEADER_MAGIC 3
typedef struct _Didj_Tex_Header {
	uint32_t magic;		// [0x000] Magic number? (always 3)
	uint32_t width_disp;	// [0x004] Width [display size]
	uint32_t height_disp;	// [0x008] Height [display size]
	uint32_t width;		// [0x00C] Width [actual size]
	uint32_t height;	// [0x010] Height [actual size]
	uint32_t uncompr_size;	// [0x014] Uncompressed data size, including palette
	uint32_t px_format;	// [0x018] Pixel format (see Didj_Pixel_Format_e)
	uint32_t num_images;	// [0x01C] Number of images? (always 1)
	uint32_t compr_size;	// [0x020] Compressed size (zlib)
} Didj_Tex_Header;
ASSERT_STRUCT(Didj_Tex_Header, 9*sizeof(uint32_t));

/**
 * Pixel format.
 */
typedef enum {
	DIDJ_PIXEL_FORMAT_RGB565	= 1,	// RGB565
	DIDJ_PIXEL_FORMAT_RGBA4444	= 3,	// RGBA4444

	DIDJ_PIXEL_FORMAT_8BPP_RGB565	= 4,	// 8bpp; palette is RGB565
	DIDJ_PIXEL_FORMAT_8BPP_RGBA4444	= 6,	// 8bpp; palette is RGBA4444

	DIDJ_PIXEL_FORMAT_4BPP_RGB565	= 7,	// 4bpp; palette is RGB565
	DIDJ_PIXEL_FORMAT_4BPP_RGBA4444	= 9,	// 4bpp; palette is RGBA4444
} Didj_Pixel_Format_e;

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_FILEFORMAT_DIDJ_TEX_STRUCTS_H__ */
