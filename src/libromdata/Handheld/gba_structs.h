/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * gba_structs.h: Nintendo Game Boy Advance data structures.               *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_GBA_STRUCTS_H__
#define __ROMPROPERTIES_LIBROMDATA_GBA_STRUCTS_H__

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Game Boy Advance ROM header.
 * This matches the GBA ROM header format exactly.
 * Reference: http://problemkaputt.de/gbatek.htm#gbacartridgeheader
 *
 * All fields are in little-endian.
 *
 * NOTE: Strings are NOT null-terminated!
 */
typedef struct _GBA_RomHeader {
	union {
		uint32_t entry_point;	// 32-bit ARM branch opcode.
		uint8_t entry_point_bytes[4];
	};
	uint8_t nintendo_logo[0x9C];	// Compressed logo.
	char title[12];
	union {
		char id6[6];	// Game code. (ID6)
		struct {
			char id4[4];		// Game code. (ID4)
			char company[2];	// Company code.
		};
	};
	uint8_t fixed_96h;	// Fixed value. (Must be 0x96)
	uint8_t unit_code;	// 0x00 for all GBA models.
	uint8_t device_type;	// 0x00. (bit 7 for debug?)
	uint8_t reserved1[7];
	uint8_t rom_version;
	uint8_t checksum;
	uint8_t reserved2[2];
} GBA_RomHeader;
ASSERT_STRUCT(GBA_RomHeader, 192);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBROMDATA_GBA_STRUCTS_H__ */
