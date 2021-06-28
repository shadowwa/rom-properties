/***************************************************************************
 * ROM Properties Page shell extension. (Win32)                            *
 * clsid_common.hpp: CLSID common macros.                                  *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_WIN32_CLSID_COMMON_HPP__
#define __ROMPROPERTIES_WIN32_CLSID_COMMON_HPP__

#include "libwin32common/RpWin32_sdk.h"
#include "libwin32common/RegKey.hpp"

// CLSID register/unregister function declarations
#define CLSID_DECL(klass) \
		/** \
		 * Register the COM object. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG RegisterCLSID(void); \
		\
		/** \
		 * Unregister the COM object. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG UnregisterCLSID(void);

// CLSID register/unregister function declarations (no inline)
#define CLSID_DECL_NOINLINE(klass) \
		/** \
		 * Register the COM object. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG RegisterCLSID(void); \
		\
		/** \
		 * Unregister the COM object. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG UnregisterCLSID(void);

// CLSID register/unregister function implementations
#define CLSID_IMPL(klass, description) \
/** \
 * Register the COM object. \
 * @return ERROR_SUCCESS on success; Win32 error code on error. \
 */ \
LONG klass::RegisterCLSID(void) \
{ \
	extern const TCHAR RP_ProgID[]; \
	\
	/* Register the COM object. */ \
	LONG lResult = RegKey::RegisterComObject(__uuidof(klass), RP_ProgID, description); \
	if (lResult != ERROR_SUCCESS) { \
		return lResult; \
	} \
	\
	/* Register as an "approved" shell extension. */ \
	return RegKey::RegisterApprovedExtension(__uuidof(klass), description); \
} \
\
/** \
 * Unregister the COM object. \
 * @return ERROR_SUCCESS on success; Win32 error code on error. \
 */ \
LONG klass::UnregisterCLSID(void) \
{ \
	extern const TCHAR RP_ProgID[]; \
	return LibWin32Common::RegKey::UnregisterComObject(__uuidof(klass), RP_ProgID); \
}


// Filetype register/unregister function declarations
#define FILETYPE_HANDLER_DECL(klass) \
		/** \
		 * Register the file type handler. \
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root. \
		 * @param ext File extension, including the leading dot. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG RegisterFileType(LibWin32Common::RegKey &hkcr, LPCTSTR ext); \
		\
		/** \
		 * Unregister the file type handler. \
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root. \
		 * @param ext File extension, including the leading dot. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG UnregisterFileType(LibWin32Common::RegKey &hkcr, LPCTSTR ext);

// Filetype register/unregister function declarations (with pHklm)
#define FILETYPE_HANDLER_HKLM_DECL(klass) \
		/** \
		 * Register the file type handler. \
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root. \
		 * @param pHklm HKEY_LOCAL_MACHINE or user-specific root, or nullptr to skip. \
		 * @param ext File extension, including the leading dot. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG RegisterFileType(LibWin32Common::RegKey &hkcr, LibWin32Common::RegKey *pHklm, LPCTSTR ext); \
		\
		/** \
		 * Unregister the file type handler. \
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root. \
		 * @param pHklm HKEY_LOCAL_MACHINE or user-specific root, or nullptr to skip. \
		 * @param ext File extension, including the leading dot. \
		 * @return ERROR_SUCCESS on success; Win32 error code on error. \
		 */ \
		static LONG UnregisterFileType(LibWin32Common::RegKey &hkcr, LibWin32Common::RegKey *pHklm, LPCTSTR ext);

#endif /* __ROMPROPERTIES_WIN32_CLSID_COMMON_HPP__ */
