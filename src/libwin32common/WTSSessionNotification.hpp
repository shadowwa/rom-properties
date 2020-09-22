/************************************************************************************
 * ROM Properties Page shell extension. (libwin32common)                            *
 * WTSSessionNotification.hpp: WTSRegisterSessionNotification() RAII wrapper class. *
 *                                                                                  *
 * Copyright (c) 2016-2019 by David Korth.                                          *
 * SPDX-License-Identifier: GPL-2.0-or-later                                        *
 ************************************************************************************/

#ifndef __ROMPROPERTIES_LIBWIN32COMMON_WTSSESSIONNOTIFICATION_HPP__
#define __ROMPROPERTIES_LIBWIN32COMMON_WTSSESSIONNOTIFICATION_HPP__

// C includes. (C++ namespace)
#include <cassert>

// Windows SDK.
#include "RpWin32_sdk.h"

namespace LibWin32Common {

/**
 * WTSRegisterSessionNotification() RAII wrapper.
 */
class WTSSessionNotification
{
	public:
		inline WTSSessionNotification()
		{
			m_hWtsApi32_dll = LoadLibrary(_T("wtsapi32.dll"));
		}

		inline ~WTSSessionNotification() {
			if (m_hWtsApi32_dll) {
				FreeLibrary(m_hWtsApi32_dll);
			}
		}

	public:
#ifndef NOTIFY_FOR_ALL_SESSIONS
# define NOTIFY_FOR_ALL_SESSIONS 1
#endif
#ifndef NOTIFY_FOR_THIS_SESSION
# define NOTIFY_FOR_THIS_SESSION 0
#endif

		/**
		 * Register for Remote Desktop session notifications.
		 * @param HWND hWnd
		 * @param dwFlags Flags
		 * @return TRUE on success; FALSE on error.
		 */
		inline BOOL registerSessionNotification(HWND hWnd, DWORD dwFlags) {
			if (!m_hWtsApi32_dll) {
				// DLL not found.
				SetLastError(ERROR_DLL_NOT_FOUND);
			}

			// Register for WTS session notifications.
			PFNWTSREGISTERSESSIONNOTIFICATION pfnWTSRegisterSessionNotification =
				(PFNWTSREGISTERSESSIONNOTIFICATION)GetProcAddress(m_hWtsApi32_dll, "WTSRegisterSessionNotification");
			if (pfnWTSRegisterSessionNotification) {
				return pfnWTSRegisterSessionNotification(hWnd, dwFlags);
			}

			// Not found in the DLL...
			// GetProcAddress() has already called SetLastError().
			return FALSE;
		}

		/**
		 * Unregister for Remote Desktop session notifications.
		 * @param HWND hWnd
		 * @return TRUE on success; FALSE on error.
		 */
		inline BOOL unRegisterSessionNotification(HWND hWnd) {
			if (!m_hWtsApi32_dll) {
				// DLL not found.
				SetLastError(ERROR_DLL_NOT_FOUND);
			}

			// Unregister for WTS session notifications.
			PFNWTSUNREGISTERSESSIONNOTIFICATION pfnWTSUnRegisterSessionNotification =
				(PFNWTSUNREGISTERSESSIONNOTIFICATION)GetProcAddress(m_hWtsApi32_dll, "WTSUnRegisterSessionNotification");
			if (pfnWTSUnRegisterSessionNotification) {
				return pfnWTSUnRegisterSessionNotification(hWnd);
			}

			// Not found in the DLL...
			// GetProcAddress() has already called SetLastError().
			return FALSE;
		}

	private:
		// Disable copying.
		WTSSessionNotification(const WTSSessionNotification &);
		WTSSessionNotification &operator=(const WTSSessionNotification &);

	private:
		// TODO: Make it static and use reference counting?
		HMODULE m_hWtsApi32_dll;
		typedef BOOL (WINAPI *PFNWTSREGISTERSESSIONNOTIFICATION)(HWND hWnd, DWORD dwFlags);
		typedef BOOL (WINAPI *PFNWTSUNREGISTERSESSIONNOTIFICATION)(HWND hWnd);
};

}

#endif /* __ROMPROPERTIES_LIBWIN32COMMON_WTSSESSIONNOTIFICATION_HPP__ */
