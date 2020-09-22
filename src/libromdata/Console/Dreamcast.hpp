/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * Dreamcast.hpp: Sega Dreamcast disc image reader.                        *
 *                                                                         *
 * Copyright (c) 2016-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_DREAMCAST_HPP__
#define __ROMPROPERTIES_LIBROMDATA_DREAMCAST_HPP__

#include "librpbase/RomData.hpp"

namespace LibRomData {

ROMDATA_DECL_BEGIN(Dreamcast)
ROMDATA_DECL_CLOSE()
ROMDATA_DECL_METADATA()
ROMDATA_DECL_IMGSUPPORT()
ROMDATA_DECL_IMGINT()
ROMDATA_DECL_END()

}

#endif /* __ROMPROPERTIES_LIBROMDATA_DREAMCAST_HPP__ */
