/***************************************************************************
 * ROM Properties Page shell extension. (Win32)                            *
 * FontHandler.cpp: Font handler.                                          *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "stdafx.h"
#include "config.librpbase.h"
#include "FontHandler.hpp"

// libwin32common
#include "libwin32common/w32err.h"

// C++ STL classes.
using std::tstring;
using std::unordered_set;
using std::vector;

class FontHandlerPrivate
{
	public:
		explicit FontHandlerPrivate(HWND hWnd);
		~FontHandlerPrivate();

	private:
		RP_DISABLE_COPY(FontHandlerPrivate)

	public:
		// Window used for the dialog font.
		HWND hWnd;

		// Monospaced font.
		HFONT hFontMono;
		LOGFONT lfFontMono;

		// Controls.
		vector<HWND> vecMonoControls;	// Controls using the monospaced font.

		// Previous ClearType setting.
		bool bPrevIsClearType;

	private:
		/**
		 * Monospaced font enumeration procedure.
		 * @param lpelfe Enumerated font information.
		 * @param lpntme Font metrics.
		 * @param FontType Font type.
		 * @param lParam Pointer to RP_ShellPropSheetExt_Private.
		 */
		static int CALLBACK MonospacedFontEnumProc(
			_In_ const LOGFONT *lpelfe,
			_In_ const TEXTMETRIC *lpntme,
			_In_ DWORD FontType,
			_In_ LPARAM lParam);

	public:
		/**
		 * Determine the monospaced font to use.
		 * @param plfFontMono Pointer to LOGFONT to store the font name in.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		static int findMonospacedFont(LOGFONT *plfFontMono);

		/**
		 * Get the current ClearType setting.
		 * @return Current ClearType setting.
		 */
		static bool isClearTypeEnabled(void);

	public:
		/**
		 * Update fonts.
		 * @param force Force update. (Use for WM_THEMECHANGED.)
		 * @return 0 on success; negative POSIX error code on error.
		 */
		int updateFonts(bool force = false);
};

/** FontHandlerPrivate **/

FontHandlerPrivate::FontHandlerPrivate(HWND hWnd)
	: hWnd(hWnd)
	, hFontMono(nullptr)
	, bPrevIsClearType(false)
{
	memset(&lfFontMono, 0, sizeof(lfFontMono));
	if (hWnd) {
		updateFonts(true);
	}
}

FontHandlerPrivate::~FontHandlerPrivate()
{
	if (hFontMono) {
		DeleteFont(hFontMono);
	}
}

/**
 * Monospaced font enumeration procedure.
 * @param lpelfe Enumerated font information.
 * @param lpntme Font metrics.
 * @param FontType Font type.
 * @param lParam Pointer to RP_ShellPropSheetExt_Private.
 */
int CALLBACK FontHandlerPrivate::MonospacedFontEnumProc(
	_In_ const LOGFONT *lpelfe,
	_In_ const TEXTMETRIC *lpntme,
	_In_ DWORD FontType,
	_In_ LPARAM lParam)
{
	((void)lpntme);
	((void)FontType);
	unordered_set<tstring> *const pFonts = reinterpret_cast<unordered_set<tstring>*>(lParam);

	// Check the font attributes:
	// - Must be monospaced.
	// - Must be horizontally-oriented.
	if ((lpelfe->lfPitchAndFamily & FIXED_PITCH) &&
	     lpelfe->lfFaceName[0] != _T('@'))
	{
		pFonts->insert(lpelfe->lfFaceName);
	}

	// Continue enumeration.
	return 1;
}

/**
 * Determine the monospaced font to use.
 * @param plfFontMono Pointer to LOGFONT to store the font name in.
 * @return 0 on success; negative POSIX error code on error.
 */
int FontHandlerPrivate::findMonospacedFont(LOGFONT *plfFontMono)
{
	// Enumerate all monospaced fonts.
	// Reference: http://www.catch22.net/tuts/fixed-width-font-enumeration
	unordered_set<tstring> enum_fonts;
#ifdef HAVE_UNORDERED_SET_RESERVE
	enum_fonts.reserve(64);
#endif /* HAVE_UNORDERED_SET_RESERVE */

	LOGFONT lfEnumFonts = {
		0,	// lfHeight
		0,	// lfWidth
		0,	// lfEscapement
		0,	// lfOrientation
		0,	// lfWeight
		0,	// lfItalic
		0,	// lfUnderline
		0,	// lfStrikeOut
		DEFAULT_CHARSET, // lfCharSet
		0,	// lfOutPrecision
		0,	// lfClipPrecision
		0,	// lfQuality
		FIXED_PITCH | FF_DONTCARE, // lfPitchAndFamily
		_T(""),	// lfFaceName
	};

	HDC hDC = GetDC(nullptr);
	EnumFontFamiliesEx(hDC, &lfEnumFonts, MonospacedFontEnumProc,
		reinterpret_cast<LPARAM>(&enum_fonts), 0);
	ReleaseDC(nullptr, hDC);

	if (enum_fonts.empty()) {
		// No fonts...
		return -ENOENT;
	}

	// Fonts to try.
	static const TCHAR *const mono_font_names[] = {
		_T("DejaVu Sans Mono"),
		_T("Consolas"),
		_T("Lucida Console"),
		_T("Fixedsys Excelsior 3.01"),
		_T("Fixedsys Excelsior 3.00"),
		_T("Fixedsys Excelsior 3.0"),
		_T("Fixedsys Excelsior 2.00"),
		_T("Fixedsys Excelsior 2.0"),
		_T("Fixedsys Excelsior 1.00"),
		_T("Fixedsys Excelsior 1.0"),
		_T("Fixedsys"),
		_T("Courier New"),
		nullptr
	};

	const TCHAR *mono_font = nullptr;
	for (const TCHAR *const *test_font = mono_font_names; *test_font != nullptr; test_font++) {
		if (enum_fonts.find(*test_font) != enum_fonts.end()) {
			// Found a font.
			mono_font = *test_font;
			break;
		}
	}

	if (!mono_font) {
		// No monospaced font found.
		return -ENOENT;
	}

	// Found the monospaced font.
	_tcscpy_s(plfFontMono->lfFaceName, _countof(plfFontMono->lfFaceName), mono_font);
	return 0;
}

/**
 * Get the current ClearType setting.
 * @return Current ClearType setting.
 */
bool FontHandlerPrivate::isClearTypeEnabled(void)
{
	// Get the current ClearType setting.
	bool bIsClearType = false;
	BOOL bFontSmoothing;
	BOOL bRet = SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &bFontSmoothing, 0);
	if (bRet) {
		UINT uiFontSmoothingType;
		bRet = SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &uiFontSmoothingType, 0);
		if (bRet) {
			bIsClearType = (bFontSmoothing && (uiFontSmoothingType == FE_FONTSMOOTHINGCLEARTYPE));
		}
	}
	return bIsClearType;
}

/**
 * Update fonts.
 * @param force Force update. (Use for WM_THEMECHANGED.)
 * @return 0 on success; negative POSIX error code on error.
 */
int FontHandlerPrivate::updateFonts(bool force)
{
	if (!hWnd) {
		// No window. Delete the fonts.
		if (hFontMono) {
			DeleteFont(hFontMono);
			hFontMono = nullptr;
		}
		return -EBADF;
	}

	// TODO: Only create fonts if controls are using them.

	// We need to create the fonts if:
	// - The fonts aren't created yet.
	// - The ClearType state was changed.
	bool bIsClearType = isClearTypeEnabled();
	bool bCreateFonts = force || (bIsClearType != bPrevIsClearType) || !hFontMono;
	if (!bCreateFonts) {
		// Nothing to do here.
		return 0;
	}

	// Create the fonts.
	HFONT hFont = GetWindowFont(hWnd);
	assert(hFont != nullptr);
	if (!hFont) {
		// Unable to get the window font.
		return -ENOENT;
	}

	if (GetObject(hFont, sizeof(lfFontMono), &lfFontMono) == 0) {
		// Unable to obtain the LOGFONT.
		return -EIO;
	}

	// Find a monospaced font.
	int ret = findMonospacedFont(&lfFontMono);
	if (ret != 0) {
		// Monospaced font not found.
		return -ENOENT;
	}

	// Create the monospaced font.
	// If ClearType is enabled, use DEFAULT_QUALITY;
	// otherwise, use NONANTIALIASED_QUALITY.
	lfFontMono.lfQuality = (bIsClearType ? DEFAULT_QUALITY : NONANTIALIASED_QUALITY);
	HFONT hFontMonoNew = CreateFontIndirect(&lfFontMono);
	if (!hFontMonoNew) {
		// Unable to create the new font.
		return -w32err_to_posix(GetLastError());
	}

	// Update all monospaced controls to use the new font.
	std::for_each(vecMonoControls.cbegin(), vecMonoControls.cend(),
		[hFontMonoNew](HWND hWnd) { SetWindowFont(hWnd, hFontMonoNew, false); }
	);

	// Delete the old font and save the new one.
	HFONT hFontMonoOld = hFontMono;
	hFontMono = hFontMonoNew;
	if (hFontMonoOld) {
		DeleteFont(hFontMonoOld);
	}

	// Update the ClearType state.
	bPrevIsClearType = bIsClearType;
	return 0;
}

/** FontHandler **/

FontHandler::FontHandler(HWND hWnd)
	: d_ptr(new FontHandlerPrivate(hWnd))
{ }

FontHandler::~FontHandler()
{
	delete d_ptr;
}

/**
 * Get the window being used for the dialog font.
 * @return Window, or nullptr on error.
 */
HWND FontHandler::window(void) const
{
	RP_D(const FontHandler);
	return d->hWnd;
}

/**
 * Set the window to use for the dialog font.
 * This will force all managed controls to be updated.
 * @param hWnd Window.
 */
void FontHandler::setWindow(HWND hWnd)
{
	RP_D(FontHandler);
	d->hWnd = hWnd;
	d->updateFonts();
}

/**
 * Get the monospaced font.
 * Needed in some cases, e.g. for ListView.
 * @return Monospaced font, or nullptr on error.
 */
HFONT FontHandler::monospacedFont(void) const
{
	RP_D(FontHandler);
	assert(d->hFontMono != nullptr);
	return d->hFontMono;
}

/**
 * Add a control that should use the monospaced font.
 * @param hWnd Control.
 */
void FontHandler::addMonoControl(HWND hWnd)
{
	RP_D(FontHandler);
	assert(d->hWnd != nullptr);
	d->vecMonoControls.emplace_back(hWnd);
	SetWindowFont(hWnd, d->hFontMono, false);
}

/**
 * Update fonts.
 * This should be called in response to:
 * - WM_NCPAINT (see below)
 * - WM_THEMECHANGED
 *
 * NOTE: This *should* be called in response to WM_SETTINGCHANGE
 * for SPI_GETFONTSMOOTHING or SPI_GETFONTSMOOTHINGTYPE, but that
 * isn't sent when previewing ClearType changes, only when applying.
 * WM_NCPAINT *is* called, though.
 *
 * @param force Force update. (Use for WM_THEMECHANGED.)
 */
void FontHandler::updateFonts(bool force)
{
	RP_D(FontHandler);
	assert(d->hWnd != nullptr);
	if (d->hWnd) {
		d->updateFonts(force);
	}
}
