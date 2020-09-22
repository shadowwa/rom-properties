/********************************************************************************
 * ROM Properties Page shell extension. (Win32)                                 *
 * RP_EmptyVolumeCacheCallback.cpp: RP_EmptyVolumeCacheCallback implementation. *
 *                                                                              *
 * Copyright (c) 2016-2017 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                                    *
 ********************************************************************************/

#include "stdafx.h"
#include "RP_EmptyVolumeCacheCallback.hpp"

RP_EmptyVolumeCacheCallback::RP_EmptyVolumeCacheCallback(HWND hProgressBar)
	: m_hProgressBar(hProgressBar)
	, m_baseProgress(0)
{ }

/** IUnknown **/
// Reference: https://msdn.microsoft.com/en-us/library/office/cc839627.aspx

IFACEMETHODIMP RP_EmptyVolumeCacheCallback::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4365 4838)
#endif /* _MSC_VER */
	static const QITAB rgqit[] = {
		QITABENT(RP_EmptyVolumeCacheCallback, IEmptyVolumeCacheCallBack),
		{ 0, 0 }
	};
#ifdef _MSC_VER
# pragma warning(pop)
#endif /* _MSC_VER */
	return LibWin32Common::rp_QISearch(this, rgqit, riid, ppvObj);
}

/** IEmptyVolumeCacheCallBack **/
// Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb762008(v=vs.85).aspx
// TODO: Needs testing on a system with lots of thumbnails.

IFACEMETHODIMP RP_EmptyVolumeCacheCallback::ScanProgress(DWORDLONG dwlSpaceUsed, DWORD dwFlags, LPCWSTR pcwszStatus)
{
	return S_OK;
}

IFACEMETHODIMP RP_EmptyVolumeCacheCallback::PurgeProgress(DWORDLONG dwlSpaceFreed, DWORDLONG dwlSpaceToFree, DWORD dwFlags, LPCWSTR pcwszStatus)
{
	// TODO: Verify that the UI gets updated here.
	if (!m_hProgressBar) {
		return S_OK;
	}
	// TODO: Better way to calculate a percentage?
	float fPct = static_cast<float>(dwlSpaceFreed) / static_cast<float>(dwlSpaceToFree);
	SendMessage(m_hProgressBar, PBM_SETSTATE, m_baseProgress + static_cast<unsigned int>(fPct * 100.0f), 0);
	return S_OK;
}
