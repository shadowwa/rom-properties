/***************************************************************************
 * ROM Properties Page shell extension. (libromdata)                       *
 * WiiSave.hpp: Nintendo Wii save game file reader.                        *
 *                                                                         *
 * Copyright (c) 2016-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBROMDATA_WIISAVE_HPP__
#define __ROMPROPERTIES_LIBROMDATA_WIISAVE_HPP__

#include "librpbase/RomData.hpp"

namespace LibRomData {

ROMDATA_DECL_BEGIN(WiiSave)
ROMDATA_DECL_CLOSE()
ROMDATA_DECL_IMGSUPPORT()
ROMDATA_DECL_IMGPF()
ROMDATA_DECL_IMGINT()
ROMDATA_DECL_ICONANIM()
ROMDATA_DECL_END()

}

#endif /* __ROMPROPERTIES_LIBROMDATA_WIISAVE_HPP__ */
