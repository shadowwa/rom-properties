/***************************************************************************
 * ROM Properties Page shell extension. (Win32)                            *
 * RP_PropertyStore.hpp: IPropertyStore implementation.                    *
 *                                                                         *
 * Copyright (c) 2016-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_WIN32_RP_PROPERTYSTORE_HPP__
#define __ROMPROPERTIES_WIN32_RP_PROPERTYSTORE_HPP__

// librpbase
#include "librpbase/config.librpbase.h"
#include "common.h"

// Reference: http://www.codeproject.com/Articles/338268/COM-in-C
#include "libwin32common/ComBase.hpp"

// CLSID
extern "C" {
	extern const CLSID CLSID_RP_PropertyStore;
}

namespace LibWin32Common {
	class RegKey;
}

class RP_PropertyStore_Private;

class UUID_ATTR("{4A1E3510-50BD-4B03-A801-E4C954F43B96}")
RP_PropertyStore final : public LibWin32Common::ComBase3<IInitializeWithStream, IPropertyStore, IPropertyStoreCapabilities>
{
	public:
		RP_PropertyStore();
	protected:
		virtual ~RP_PropertyStore();

	private:
		typedef LibWin32Common::ComBase3<IInitializeWithStream, IPropertyStore, IPropertyStoreCapabilities> super;
		RP_DISABLE_COPY(RP_PropertyStore)
	private:
		friend class RP_PropertyStore_Private;
		RP_PropertyStore_Private *const d_ptr;

	public:
		// IUnknown
		IFACEMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj) final;

	private:
		/**
		 * Get the PreviewDetails string.
		 * @return PreviewDetails string.
		 */
		static std::tstring GetPreviewDetailsString();

		/**
		 * Get the InfoTip string.
		 * @return InfoTip string.
		 */
		static std::tstring GetInfoTipString();

		/**
		 * Get the FullDetails string.
		 * @return FullDetails string.
		 */
		static std::tstring GetFullDetailsString();

	public:
		/**
		 * Register the COM object.
		 * @return ERROR_SUCCESS on success; Win32 error code on error.
		 */
		static LONG RegisterCLSID(void);

		/**
		 * Register the file type handler.
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root.
		 * @param pHklm HKEY_LOCAL_MACHINE or user-specific root, or nullptr to skip.
		 * @param ext File extension, including the leading dot.
		 * @return ERROR_SUCCESS on success; Win32 error code on error.
		 */
		static LONG RegisterFileType(LibWin32Common::RegKey &hkcr, LibWin32Common::RegKey *pHklm, LPCTSTR ext);

		/**
		 * Unregister the COM object.
		 * @return ERROR_SUCCESS on success; Win32 error code on error.
		 */
		static LONG UnregisterCLSID(void);

		/**
		 * Unregister the file type handler.
		 * @param hkcr HKEY_CLASSES_ROOT or user-specific classes root.
		 * @param pHklm HKEY_LOCAL_MACHINE or user-specific root, or nullptr to skip.
		 * @param ext File extension, including the leading dot.
		 * @return ERROR_SUCCESS on success; Win32 error code on error.
		 */
		static LONG UnregisterFileType(LibWin32Common::RegKey &hkcr, LibWin32Common::RegKey *pHklm, LPCTSTR ext);

	public:
		// IInitializeWithStream
		IFACEMETHODIMP Initialize(IStream *pstream, DWORD grfMode) final;

		// IPropertyStore
		IFACEMETHODIMP Commit(void) final;
		IFACEMETHODIMP GetAt(_In_ DWORD iProp, _Out_ PROPERTYKEY *pkey) final;
		IFACEMETHODIMP GetCount(_Out_ DWORD *cProps) final;
		IFACEMETHODIMP GetValue(_In_ REFPROPERTYKEY key, _Out_ PROPVARIANT *pv) final;
		IFACEMETHODIMP SetValue(_In_ REFPROPERTYKEY key, _In_ REFPROPVARIANT propvar) final;

		// IPropertyStoreCapabilities
		IFACEMETHODIMP IsPropertyWritable(REFPROPERTYKEY key) final;
};

#ifdef __CRT_UUID_DECL
// Required for MinGW-w64 __uuidof() emulation.
__CRT_UUID_DECL(RP_PropertyStore, __MSABI_LONG(0x4a1e3510), 0x50bd, 0x4b03, 0xa8,0x01, 0xe4, 0xc9, 0x54, 0xf4, 0x3b, 0x96)
#endif

#endif /* __ROMPROPERTIES_WIN32_RP_PROPERTYSTORE_HPP__ */
