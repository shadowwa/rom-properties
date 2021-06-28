/***************************************************************************
 * ROM Properties Page shell extension. (Win32)                            *
 * RP_PropertyStore.hpp: IPropertyStore implementation.                    *
 *                                                                         *
 * Copyright (c) 2016-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_WIN32_RP_PROPERTYSTORE_HPP__
#define __ROMPROPERTIES_WIN32_RP_PROPERTYSTORE_HPP__

// librpbase
#include "librpbase/config.librpbase.h"
#include "common.h"

// Reference: http://www.codeproject.com/Articles/338268/COM-in-C
#include "libwin32common/ComBase.hpp"

// CLSID common macros
#include "clsid_common.hpp"

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
		CLSID_DECL(RP_PropertyStore)
		FILETYPE_HANDLER_HKLM_DECL(RP_PropertyStore)

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
		// IUnknown
		IFACEMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj) final;

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
