/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * Nintendo3DSFirmData.hpp: Nintendo 3DS firmware data.                    *
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

// GameTDB uses ID6 for retail Wii U titles. The publisher ID
// is NOT stored in the plaintext .wud header, so it's not
// possible to use GameTDB unless we hard-code all of the
// publisher IDs here.

#ifndef __ROMPROPERTIES_LIBROMDATA_NINTENDO3DSFIRMDATA_HPP__
#define __ROMPROPERTIES_LIBROMDATA_NINTENDO3DSFIRMDATA_HPP__

#include "librpbase/config.librpbase.h"
#include "librpbase/common.h"

namespace LibRomData {

class Nintendo3DSFirmData
{
	private:
		// Static class.
		Nintendo3DSFirmData();
		~Nintendo3DSFirmData();
		RP_DISABLE_COPY(Nintendo3DSFirmData)

	public:
		struct FirmBin_t {
			uint32_t crc;		// FIRM CRC32.
			uint8_t major;
			uint8_t minor;
			uint8_t revision;
			bool isNew3DS;		// Is this New3DS?
			char version[8];	// Display version.
		};

		/**
		 * Look up a Nintendo 3DS firmware binary.
		 * @param Firmware binary CRC32.
		 * @return Firmware binary data, or nullptr if not found.
		 */
		static const FirmBin_t *lookup_firmBin(uint32_t crc);
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_NINTENDO3DSFIRMDATA_HPP__ */
