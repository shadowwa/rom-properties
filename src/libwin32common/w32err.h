/***************************************************************************
 * ROM Properties Page shell extension. (libwin32common)                   *
 * w32err.c: Error code mapping. (Windows to POSIX)                        *
 *                                                                         *
 * Copyright (c) 2016-2017 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBWIN32COMMON_W32ERR_H__
#define __ROMPROPERTIES_LIBWIN32COMMON_W32ERR_H__

#include "RpWin32_sdk.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a Win32 error code to a POSIX error code.
 * @param w32err Win32 error code.
 * @return Positive POSIX error code. (If no equivalent is found, default is EINVAL.)
 */
int w32err_to_posix(DWORD w32err);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBWIN32COMMON_W32ERR_H__ */
