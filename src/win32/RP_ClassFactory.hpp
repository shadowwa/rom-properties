/***************************************************************************
 * ROM Properties Page shell extension. (Win32)                            *
 * RP_ClassFactory.hpp: IClassFactory implementation.                      *
 *                                                                         *
 * Copyright (c) 2016-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __ROMPROPERTIES_WIN32_RP_CLASSFACTORY_HPP__
#define __ROMPROPERTIES_WIN32_RP_CLASSFACTORY_HPP__

// References:
// - http://www.codeproject.com/Articles/665/A-very-simple-COM-server-without-ATL-or-MFC
// - http://www.codeproject.com/Articles/338268/COM-in-C

#include "libwin32common/ComBase.hpp"

template<class comObj>
class RP_MultiCreator
{
	protected:
		comObj *CreateObject()
		{
			return new comObj;
		}
};

template <class comObj, class creatorClass = RP_MultiCreator<comObj> >
class RP_ClassFactory final : public LibWin32Common::ComBase<IClassFactory>, public creatorClass
{
	public:
		RP_ClassFactory() { }

	private:
		typedef LibWin32Common::ComBase<IClassFactory> super;
		RP_DISABLE_COPY(RP_ClassFactory)

	public:
		/** IUnknown **/

		IFACEMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObject) final
		{
			if (!ppvObject) {
				return E_POINTER;
			}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4365 4838)
#endif /* _MSC_VER */
			static const QITAB rgqit[] = {
				QITABENT(RP_ClassFactory, IClassFactory),
				{ 0, 0 }
			};
#ifdef _MSC_VER
# pragma warning(pop)
#endif /* _MSC_VER */
			return LibWin32Common::rp_QISearch(this, rgqit, riid, ppvObject);
		}

		/** IClassFactory **/

		IFACEMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObject) final
		{
			// Always set out parameter to NULL, validating it first.
			if (!ppvObject)
				return E_INVALIDARG;
			*ppvObject = NULL;

			if (pUnkOuter) {
				// Aggregation is not supported.
				return CLASS_E_NOAGGREGATION;
			}

			// Create an instance of the object.
			comObj *pObj = RP_MultiCreator<comObj>::CreateObject();
			if (!pObj) {
				// Could not create the object.
				return E_OUTOFMEMORY;
			}

			// Query the object for the requested interface.
			HRESULT hr = pObj->QueryInterface(riid, ppvObject);
			pObj->Release();
			return hr;
		}

		IFACEMETHODIMP LockServer(BOOL fLock) final
		{
			CoLockObjectExternal(this, fLock, TRUE);
			return S_OK;
		}
};

#endif /* __ROMPROPERTIES_WIN32_RP_CLASSFACTORY_HPP__ */
