/***************************************************************************
 * ROM Properties Page shell extension. (libwin32common)                   *
 * QITab.h: QITAB header.                                                  *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_LIBWIN32COMMON_SDK_QITAB_H__
#define __ROMPROPERTIES_LIBWIN32COMMON_SDK_QITAB_H__

#include "../RpWin32_sdk.h"
#include <shlwapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OFFSETOFCLASS
// Get the class vtable offset. Used for QITAB.
// NOTE: QITAB::dwOffset is int, not DWORD.
#define OFFSETOFCLASS(base, derived) \
    (int)((DWORD)(DWORD_PTR)((base*)((derived*)8))-8)
#endif

#ifndef QITABENT
// QITAB is not defined on MinGW-w64 4.0.6.
typedef struct {
	const IID *piid;
	int dwOffset;
} QITAB, *LPQITAB;
typedef const QITAB *LPCQITAB;

#ifdef __cplusplus
# define QITABENTMULTI(Cthis, Ifoo, Iimpl) \
    { &__uuidof(Ifoo), OFFSETOFCLASS(Iimpl, Cthis) }
#else
# define QITABENTMULTI(Cthis, Ifoo, Iimpl) \
    { (IID*) &IID_##Ifoo, OFFSETOFCLASS(Iimpl, Cthis) }
#endif /* __cplusplus */

#define QITABENTMULTI2(Cthis, Ifoo, Iimpl) \
    { (IID*) &Ifoo, OFFSETOFCLASS(Iimpl, Cthis) }

#define QITABENT(Cthis, Ifoo) QITABENTMULTI(Cthis, Ifoo, Ifoo)
#endif /* QITABENT */

// QISearch() function pointer.
typedef HRESULT (STDAPICALLTYPE *PFNQISEARCH)(_Inout_ void *that, _In_ LPCQITAB pqit, _In_ REFIID riid, _COM_Outptr_ void **ppv);

#ifdef __cplusplus
}
#endif

#endif /* __ROMPROPERTIES_LIBWIN32COMMON_SDK_QITAB_H__ */
