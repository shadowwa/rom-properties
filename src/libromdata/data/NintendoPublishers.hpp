/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * NintendoPublishers.cpp: Nintendo third-party publishers list.           *
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

#ifndef __ROMPROPERTIES_LIBROMDATA_NINTENDOPUBLISHERS_HPP__
#define __ROMPROPERTIES_LIBROMDATA_NINTENDOPUBLISHERS_HPP__

#include "librpbase/config.librpbase.h"
#include "librpbase/common.h"

// C includes.
#include <stdint.h>

namespace LibRomData {

class NintendoPublishers
{
	private:
		NintendoPublishers();
		~NintendoPublishers();
	private:
		RP_DISABLE_COPY(NintendoPublishers)

	private:
		/**
		 * Nintendo third-party publisher list.
		 * References:
		 * - http://www.gametdb.com/Wii
		 * - http://www.gametdb.com/Wii/Downloads
		 */
		struct ThirdPartyList {
			uint16_t code;			// 2-byte code
			const char *publisher;
		};
		static const ThirdPartyList ms_thirdPartyList[];

	private:
		/**
		 * Comparison function for bsearch().
		 * @param a
		 * @param b
		 * @return
		 */
		static int RP_C_API compar(const void *a, const void *b);

	public:
		/**
		 * Look up a company code.
		 * @param code Company code.
		 * @return Publisher, or nullptr if not found.
		 */
		static const char *lookup(uint16_t code);

		/**
		 * Look up a company code.
		 * @param code Company code.
		 * @return Publisher, or nullptr if not found.
		 */
		static const char *lookup(const char *code);

		/**
		 * Look up a company code.
		 * This uses the *old* company code, present in
		 * older Game Boy titles.
		 * @param code Company code.
		 * @return Publisher, or nullptr if not found.
		 */
		static const char *lookup_old(uint8_t code);
};

}

#endif /* __ROMPROPERTIES_LIBROMDATA_NINTENDOPUBLISHERS_HPP__ */
