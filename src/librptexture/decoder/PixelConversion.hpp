/***************************************************************************
 * ROM Properties Page shell extension. (librptexture)                     *
 * PixelConversion.hpp: Pixel conversion inline functions.                 *
 *                                                                         *
 * Copyright (c) 2016-2019 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBRPTEXTURE_DECODER_PIXELCONVERSION_HPP__
#define __ROMPROPERTIES_LIBRPTEXTURE_DECODER_PIXELCONVERSION_HPP__

#include "byteswap.h"

// C includes. (C++ namespace)
#include <cmath>

namespace LibRpTexture { namespace PixelConversion {

/** Color conversion functions. **/
// NOTE: px16 and px32 are always in host-endian.

/** Lookup tables. **/
// 2-bit alpha lookup table.
extern const uint32_t a2_lookup[4];
// 3-bit alpha lookup table.
extern const uint32_t a3_lookup[8];
// 2-bit color lookup table.
extern const uint8_t c2_lookup[4];
// 3-bit color lookup table.
extern const uint8_t c3_lookup[8];

// 16-bit RGB

/**
 * Convert an RGB565 pixel to ARGB32.
 * @param px16 RGB565 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB565_to_ARGB32(uint16_t px16)
{
	// RGB565: RRRRRGGG GGGBBBBB
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 << 8) & 0xF80000) |	// Red
		((px16 << 3) & 0x0000F8);	// Blue
	px32 |=  (px32 >> 5) & 0x070007;	// Expand from 5-bit to 8-bit
	// Green
	px32 |= (((px16 <<  5) & 0x00FC00) | ((px16 >>  1) & 0x000300));
	return px32;
}

/**
 * Convert a BGR565 pixel to ARGB32.
 * @param px16 BGR565 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t BGR565_to_ARGB32(uint16_t px16)
{
	// RGB565: BBBBBGGG GGGRRRRR
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 << 19) & 0xF80000) |	// Red
		((px16 >>  8) & 0x0000F8);	// Blue
	px32 |=  (px32 >>  5) & 0x070007;	// Expand from 5-bit to 8-bit
	// Green
	px32 |= (((px16 <<  5) & 0x00FC00) | ((px16 >>  1) & 0x000300));
	return px32;
}

/**
 * Convert an ARGB1555 pixel to ARGB32.
 * @param px16 ARGB1555 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ARGB1555_to_ARGB32(uint16_t px16)
{
	// ARGB1555: ARRRRRGG GGGBBBBB
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32 = (((px16 << 9) & 0xF80000)) |	// Red
	       (((px16 << 6) & 0x00F800)) |	// Green
	       (((px16 << 3) & 0x0000F8));	// Blue
	px32 |= ((px32 >> 5) & 0x070707);	// Expand from 5-bit to 8-bit
	// Alpha channel.
	if (px16 & 0x8000) {
		px32 |= 0xFF000000U;
	}
	return px32;
}

/**
 * Convert an ABGR1555 pixel to ARGB32.
 * @param px16 ABGR1555 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ABGR1555_to_ARGB32(uint16_t px16)
{
	// ABGR1555: ABBBBBGG GGGRRRRR
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32 = (((px16 << 19) & 0xF80000)) |	// Red
	       (((px16 <<  6) & 0x00F800)) |	// Green
	       (((px16 >>  7) & 0x0000F8));	// Blue
	px32 |= ((px32 >>  5) & 0x070707);	// Expand from 5-bit to 8-bit
	// Alpha channel.
	if (px16 & 0x8000) {
		px32 |= 0xFF000000U;
	}
	return px32;
}

/**
 * Convert an RGBA5551 pixel to ARGB32.
 * @param px16 RGBA5551 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGBA5551_to_ARGB32(uint16_t px16)
{
	// RGBA5551: RRRRRGGG GGBBBBBA
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32 = (((px16 << 8) & 0xF80000)) |	// Red
	       (((px16 << 5) & 0x00F800)) |	// Green
	       (((px16 << 2) & 0x0000F8));	// Blue
	px32 |= ((px32 >> 5) & 0x070707);	// Expand from 5-bit to 8-bit
	// Alpha channel.
	if (px16 & 0x0001) {
		px32 |= 0xFF000000U;
	}
	return px32;
}

/**
 * Convert a BGRA5551 pixel to ARGB32.
 * @param px16 BGRA5551 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t BGRA5551_to_ARGB32(uint16_t px16)
{
	// BGRA5551: BBBBBGGG GGRRRRRA
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32 = (((px16 << 18) & 0xF80000)) |	// Red
	       (((px16 <<  5) & 0x00F800)) |	// Green
	       (((px16 >>  8) & 0x0000F8));	// Blue
	px32 |= ((px32 >>  5) & 0x070707);	// Expand from 5-bit to 8-bit
	// Alpha channel.
	if (px16 & 0x0001) {
		px32 |= 0xFF000000U;
	}
	return px32;
}

/**
 * Convert an ARGB4444 pixel to ARGB32.
 * @param px16 ARGB4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ARGB4444_to_ARGB32(uint16_t px16)
{
	// ARGB4444: AAAARRRR GGGGBBBB
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32  =  (px16 & 0x000F);		// B
	px32 |= ((px16 & 0x00F0) << 4);		// G
	px32 |= ((px16 & 0x0F00) << 8);		// R
	px32 |= ((px16 & 0xF000) << 12);	// A
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an ABGR4444 pixel to ARGB32.
 * @param px16 ABGR4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ABGR4444_to_ARGB32(uint16_t px16)
{
	// ARGB4444: AAAABBBB GGGGRRRR
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32  = ((px16 & 0x000F) << 16);	// R
	px32 |= ((px16 & 0x00F0) << 4);		// G
	px32 |= ((px16 & 0x0F00) >> 8);		// B
	px32 |= ((px16 & 0xF000) << 12);	// A
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an RGBA4444 pixel to ARGB32.
 * @param px16 RGBA4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGBA4444_to_ARGB32(uint16_t px16)
{
	// RGBA4444: RRRRGGGG BBBBAAAA
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32  = ((px16 & 0x000F) << 24);	// A
	px32 |= ((px16 & 0x00F0) >> 4);		// B
	px32 |=  (px16 & 0x0F00);		// G
	px32 |= ((px16 & 0xF000) << 4);		// R
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert a BGRA4444 pixel to ARGB32.
 * @param px16 BGRA4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t BGRA4444_to_ARGB32(uint16_t px16)
{
	// RGBA4444: BBBBGGGG RRRRAAAA
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32  = ((px16 & 0x000F) << 24);	// A
	px32 |= ((px16 & 0x00F0) << 12);	// R
	px32 |=  (px16 & 0x0F00);		// G
	px32 |= ((px16 & 0xF000) >> 12);	// B
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an xRGB4444 pixel to ARGB32.
 * @param px16 xRGB4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t xRGB4444_to_ARGB32(uint16_t px16)
{
	// xRGB4444: xxxxRRRR GGGGBBBB
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |=  (px16 & 0x000F);		// B
	px32 |= ((px16 & 0x00F0) << 4);		// G
	px32 |= ((px16 & 0x0F00) << 8);		// R
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an xBGR4444 pixel to ARGB32.
 * @param px16 xBGR4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t xBGR4444_to_ARGB32(uint16_t px16)
{
	// xRGB4444: xxxxBBBB GGGGRRRR
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 & 0x000F) << 16);	// R
	px32 |= ((px16 & 0x00F0) << 4);		// G
	px32 |= ((px16 & 0x0F00) >> 8);		// B
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an RGBx4444 pixel to ARGB32.
 * @param px16 RGBx4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGBx4444_to_ARGB32(uint16_t px16)
{
	// RGBx4444: RRRRGGGG BBBBxxxx
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 & 0x00F0) >> 4);		// B
	px32 |=  (px16 & 0x0F00);		// G
	px32 |= ((px16 & 0xF000) << 4);		// R
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert a BGRx4444 pixel to ARGB32.
 * @param px16 BGRx4444 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t BGRx4444_to_ARGB32(uint16_t px16)
{
	// RGBx4444: BBBBGGGG RRRRxxxx
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 & 0x00F0) << 12);	// R
	px32 |=  (px16 & 0x0F00);		// G
	px32 |= ((px16 & 0xF000) >> 12);	// B
	px32 |=  (px32 << 4);			// Copy to the top nybble.
	return px32;
}

/**
 * Convert an ARGB8332 pixel to ARGB32.
 * @param px16 ARGB8332 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ARGB8332_to_ARGB32(uint16_t px16)
{
	// ARGB8332: AAAAAAAA RRRGGGBB
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32;
	px32 = (c3_lookup[(px16 >> 5) & 7] << 16) |	// Red
	       (c3_lookup[(px16 >> 2) & 7] <<  8) |	// Green
	       (c2_lookup[ px16       & 3]      ) |	// Blue
	       ((px16 << 16) & 0xFF000000);		// Alpha
	return px32;
}

/**
 * Convert an RG88 pixel to ARGB32.
 * @param px16 RG88 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RG88_to_ARGB32(uint16_t px16)
{
	// RG88:     RRRRRRRR GGGGGGGG
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	return 0xFF000000 | (static_cast<uint32_t>(px16) << 8);
}

/**
 * Convert a GR88 pixel to ARGB32.
 * @param px16 GR88 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t GR88_to_ARGB32(uint16_t px16)
{
	// GR88:     GGGGGGGG RRRRRRRR
	// ARGB32:   AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	return 0xFF000000 | (static_cast<uint32_t>(__swab16(px16)) << 8);
}

// GameCube-specific 16-bit RGB

/**
 * Convert an RGB5A3 pixel to ARGB32. (GameCube/Wii)
 * @param px16 RGB5A3 pixel. (Must be host-endian.)
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB5A3_to_ARGB32(uint16_t px16)
{
	uint32_t px32;

	if (px16 & 0x8000) {
		// RGB555: xRRRRRGG GGGBBBBB
		// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		px32  = 0xFF000000U;	// no alpha channel
		px32 |= ((px16 << 3) & 0x0000F8);	// Blue
		px32 |= ((px16 << 6) & 0x00F800);	// Green
		px32 |= ((px16 << 9) & 0xF80000);	// Red
		px32 |= ((px32 >> 5) & 0x070707);	// Expand from 5-bit to 8-bit
	} else {
		// RGB4A3: xAAARRRR GGGGBBBB
		// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		px32  =  (px16 & 0x000F);	// Blue
		px32 |= ((px16 & 0x00F0) << 4);	// Green
		px32 |= ((px16 & 0x0F00) << 8);	// Red
		px32 |= (px32 << 4);		// Copy to the top nybble.

		// Calculate and apply the alpha channel.
		px32 |= a3_lookup[((px16 >> 12) & 0x07)];
	}

	return px32;
}

/**
 * Convert an IA8 pixel to ARGB32. (GameCube/Wii)
 * NOTE: Uses a grayscale palette.
 * @param px16 IA8 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t IA8_to_ARGB32(uint16_t px16)
{
	// FIXME: What's the component order of IA8?
	// Assuming I=MSB, A=LSB...

	// IA8:    IIIIIIII AAAAAAAA
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t i = (px16 & 0xFF00);
	i |= (i << 8);
	i |= (i >> 8);
	i |= ((px16 & 0x00FF) << 24);
	return i;
}

// Nintendo 3DS-specific 16-bit RGB

/**
 * Convert an RGB565+A4 pixel to ARGB32.
 * @param px16 RGB565 pixel.
 * @param a4 A4 value.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB565_A4_to_ARGB32(uint16_t px16, uint8_t a4)
{
	// RGB565: RRRRRGGG GGGBBBBB
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	a4 &= 0x0F;
	uint32_t px32 = (a4 << 24) | (a4 << 28);	// Alpha
	px32 |= ((px16 << 8) & 0xF80000) |	// Red
		((px16 << 3) & 0x0000F8);	// Blue
	px32 |=  (px32 >> 5) & 0x070007;	// Expand from 5-bit to 8-bit
	// Green
	px32 |= (((px16 <<  5) & 0x00FC00) | ((px16 >>  1) & 0x000300));
	return px32;
}

// PlayStation 2-specific 16-bit RGB

/**
 * Convert a BGR5A3 pixel to ARGB32. (PlayStation 2)
 * Similar to GameCube RGB5A3, but the R and B channels are swapped.
 * @param px16 BGR5A3 pixel. (Must be host-endian.)
 * @return ARGB32 pixel.
 */
static inline uint32_t BGR5A3_to_ARGB32(uint16_t px16)
{
	uint32_t px32;

	if (px16 & 0x8000) {
		// BGR555: xBBBBBGG GGGRRRRR
		// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		px32  = 0xFF000000U;	// no alpha channel
		px32 |= ((px16 >>  7) & 0x0000F8);	// Blue
		px32 |= ((px16 <<  6) & 0x00F800);	// Green
		px32 |= ((px16 << 19) & 0xF80000);	// Red
		px32 |= ((px32 >>  5) & 0x070707);	// Expand from 5-bit to 8-bit
	} else {
		// BGR4A3: xAAABBBB GGGGRRRR
		// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		px32  = ((px16 & 0x0F00) >>  8);	// Blue
		px32 |= ((px16 & 0x00F0) <<  4);	// Green
		px32 |= ((px16 & 0x000F) << 16);	// Red
		px32 |= (px32 << 4);		// Copy to the top nybble.

		// Calculate and apply the alpha channel.
		px32 |= a3_lookup[((px16 >> 12) & 0x07)];
	}

	return px32;
}

// 15-bit RGB

/**
 * Convert an RGB555 pixel to ARGB32.
 * @param px16 RGB555 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB555_to_ARGB32(uint16_t px16)
{
	// RGB555: xRRRRRGG GGGBBBBB
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 << 9) & 0xF80000) |	// Red
		((px16 << 6) & 0x00F800) |	// Green
		((px16 << 3) & 0x0000F8);	// Blue
	px32 |= ((px32 >> 5) & 0x070707);	// Expand from 5-bit to 8-bit
	return px32;
}

/**
 * Convert a BGR555 pixel to ARGB32.
 * @param px16 BGR555 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t BGR555_to_ARGB32(uint16_t px16)
{
	// BGR555: xBBBBBGG GGGRRRRR
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t px32 = 0xFF000000U;
	px32 |= ((px16 << 19) & 0xF80000) |	// Red
		((px16 <<  6) & 0x00F800) |	// Green
		((px16 >>  7) & 0x0000F8);	// Blue
	px32 |= ((px32 >>  5) & 0x070707);	// Expand from 5-bit to 8-bit
	return px32;
}

// 32-bit RGB

/**
 * Convert a G16R16 pixel to ARGB32.
 * @param px32 G16R16 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t G16R16_to_ARGB32(uint32_t px32)
{
	// NOTE: This will truncate the color channels.
	// TODO: Add ARGB64 support?

	// G16R16: GGGGGGGG gggggggg RRRRRRRR rrrrrrrr
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb = 0xFF000000U;
	argb |= ((px32 <<  8) & 0x00FF0000) |
		((px32 >> 16) & 0x0000FF00);
	return argb;
}

/**
 * Convert an A2R10G10B10 pixel to ARGB32.
 * @param px32 A2R10G10B10 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t A2R10G10B10_to_ARGB32(uint32_t px32)
{
	// NOTE: This will truncate the color channels.
	// TODO: Add ARGB64 support?

	// A2R10G10B10: AARRRRRR RRrrGGGG GGGGggBB BBBBBBbb
	//      ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb;
	argb = ((px32 >> 6) & 0xFF0000) |	// Red
	       ((px32 >> 4) & 0x00FF00) |	// Green
	       ((px32 >> 2) & 0x0000FF) |	// Blue
	       a2_lookup[px32 >> 30];		// Alpha
	return argb;
}

/**
 * Convert an A2B10G10R10 pixel to ARGB32.
 * @param px32 A2B10G10R10 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t A2B10G10R10_to_ARGB32(uint32_t px32)
{
	// NOTE: This will truncate the color channels.
	// TODO: Add ARGB64 support?

	// A2B10G10R10: AABBBBBB BBbbGGGG GGGGggRR RRRRRRrr
	//      ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb;
	argb = ((px32 << 14) & 0xFF0000) |	// Red
	       ((px32 >>  4) & 0x00FF00) |	// Green
	       ((px32 >> 22) & 0x0000FF) |	// Blue
	       a2_lookup[px32 >> 30];		// Alpha
	return argb;
}

/**
 * Convert an RGB9_E5 pixel to ARGB32.
 * NOTE: RGB9_E5 is an HDR format; this converts to LDR.
 * @param px32 A2B10G10R10 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB9_E5_to_ARGB32(uint32_t px32)
{
	// NOTE: This will truncate the color channels.
	// TODO: Add ARGB64 support?

	// Reference: https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt
	// RGB9_E5: EEEEEBBB BBBBBBGG GGGGGGGR RRRRRRRR
	//  ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	int8_t e = (px32 >> 27) - 15 - 9;

	// Process as float.
	// TODO: SSE optimizations?
	float rf =  (px32        & 0x1FF) * powf(2, e);
	float gf = ((px32 >>  9) & 0x1FF) * powf(2, e);
	float bf = ((px32 >> 18) & 0x1FF) * powf(2, e);

	// Convert to uint8_t, clamping to [0,255].
	uint8_t r = (rf <= 0.0f ? 0 : (rf >= 1.0f ? 255 : ((uint8_t)floorf(rf * 256.0f))));
	uint8_t g = (gf <= 0.0f ? 0 : (gf >= 1.0f ? 255 : ((uint8_t)floorf(gf * 256.0f))));
	uint8_t b = (bf <= 0.0f ? 0 : (bf >= 1.0f ? 255 : ((uint8_t)floorf(bf * 256.0f))));

	// Convert back to ARGB32.
	return (0xFF000000 | (r << 16) | (g << 8) | b);
}

// PlayStation 2-specific 32-bit RGB

/**
 * Convert a BGR888_ABGR7888 pixel to ARGB32. (PlayStation 2)
 * Similar to GameCube RGB5A3, but with 32-bit channels.
 * (Why would you do this... Just set alpha to 0xFF!)
 * @param px16 BGR888_ABGR7888 pixel. (Must be host-endian.)
 * @return ARGB32 pixel.
 */
static inline uint32_t BGR888_ABGR7888_to_ARGB32(uint32_t px32)
{
	uint32_t argb;

	if (px32 & 0x80000000U) {
		// BGR888: xxxxxxxx BBBBBBBB GGGGGGGG RRRRRRRR
		// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		argb = 0xFF000000U;		// no alpha channel
		argb |= (px32 >> 16) & 0xFF;	// Blue
		argb |= (px32 & 0x0000FF00U);	// Green
		argb |= (px32 & 0xFF) << 16;	// Red
	} else {
		// ABGR7888: xAAAAAAA BBBBBBBB GGGGGGGG RRRRRRRR
		//   ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
		argb  = (px32 & 0x7F000000U) << 1;	// Alpha
		argb |= (argb & 0x80000000U) >> 7;	// Alpha LSB
		argb |= (px32 >> 16) & 0xFF;	// Blue
		argb |= (px32 & 0x0000FF00U);	// Green
		argb |= (px32 & 0xFF) << 16;	// Red
	}

	return argb;
}

// Luminance

/**
 * Convert an L8 pixel to ARGB32.
 * NOTE: Uses a grayscale palette.
 * @param px8 L8 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t L8_to_ARGB32(uint8_t px8)
{
	//     L8: LLLLLLLL
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb = 0xFF000000U;
	argb |= px8 | (px8 << 8) | (px8 << 16);
	return argb;
}

/**
 * Convert an A4L4 pixel to ARGB32.
 * NOTE: Uses a grayscale palette.
 * @param px8 A4L4 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t A4L4_to_ARGB32(uint8_t px8)
{
	//   A4L4: AAAALLLL
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb;
	argb = ((px8 & 0xF0) << 20) | (px8 & 0x0F);	// Low nybble of A and B.
	argb |= (argb << 4);				// Copy to high nybble.
	argb |= (argb & 0xFF) <<  8;			// Copy B to G.
	argb |= (argb & 0xFF) << 16;			// Copy B to R.
	return argb;
}

/**
 * Convert an L16 pixel to ARGB32.
 * NOTE: Uses a grayscale palette.
 * @param px16 L16 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t L16_to_ARGB32(uint16_t px16)
{
	// NOTE: This will truncate the luminance.
	// TODO: Add ARGB64 support?
	return L8_to_ARGB32(px16 >> 8);
}

/**
 * Convert an A8L8 pixel to ARGB32.
 * NOTE: Uses a grayscale palette.
 * @param px16 A8L8 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t A8L8_to_ARGB32(uint16_t px16)
{
	//   A8L8: AAAAAAAA LLLLLLLL
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb;
	argb =  (px16 & 0xFF) |			// Red
	       ((px16 & 0xFF) << 8) |		// Green
	       ((px16 & 0xFF) << 16) |		// Blue
	       ((px16 << 16) & 0xFF000000);	// Alpha
	return argb;
}

/**
 * Convert an L8A8 pixel to ARGB32.
 * NOTE: Uses a grayscale palette.
 * @param px16 A8L8 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t L8A8_to_ARGB32(uint16_t px16)
{
	//   L8A8: LLLLLLLL AAAAAAAA
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	uint32_t argb;
	argb =  (px16 >> 8) |			// Red
	        (px16 & 0xFF00) |		// Green
	       ((px16 << 8) & 0x00FF0000) |	// Blue
	       ((px16 & 0xFF) << 24);		// Alpha
	return argb;
}

// Alpha

/**
 * Convert an A8 pixel to ARGB32.
 * NOTE: Uses a black background.
 * @param px8 A8 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t A8_to_ARGB32(uint8_t px8)
{
	//     A8: AAAAAAAA
	// ARGB32: AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
	return (px8 << 24);
}

} }

#endif /* __ROMPROPERTIES_LIBRPTEXTURE_DECODER_PIXELCONVERSION_HPP__ */
