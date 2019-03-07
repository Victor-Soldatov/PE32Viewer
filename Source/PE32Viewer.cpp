// PE32Viewer.cpp : Implementation of DLL Exports.

#pragma warning( disable : 4995 )

#include "stdafx.h"
#include "resource.h"
#include "PE32Viewer_i.h"
#include "dllmain.h"
#include "xdlldata.h"
#include "PE32Viewer.h"

const __wchar_t wszRegKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";
const __wchar_t wszAppID[] = L"PE32Viewer.CoPE32Viewer";
const __wchar_t wszAppDisplayName[] = L"PE32 structure viewer";

#include <memory>

volatile unsigned long g_nCount;

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void)
{
	if (0 != ::InterlockedCompareExchange(&g_nCount, 0, 0))
		return S_FALSE;

#ifdef _MERGE_PROXYSTUB
	HRESULT hr = PrxDllCanUnloadNow();
	if (hr != S_OK)
		return hr;
#endif

	return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif
		return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();

	CLSID ClassID = { 0 };
	if (FAILED(::CLSIDFromProgID(&wszAppID[0], &ClassID)))
		return E_ACCESSDENIED;

	LPOLESTR lpszCLSID(nullptr);
	__wchar_t szCLSID[64] = { 0 };
	if (FAILED(::StringFromCLSID(ClassID, &lpszCLSID)))
		return E_ACCESSDENIED;
	else
	{
		::StringCchCopyW(szCLSID, 64, lpszCLSID);
		::CoTaskMemFree(lpszCLSID);
	}

	CXRegKey KeyApproved;
	HKEY hKeyApproved(nullptr); 

	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, wszRegKey, 0, KEY_SET_VALUE, &hKeyApproved))
		return E_ACCESSDENIED;
	else
		KeyApproved.Assign(hKeyApproved);

	if (ERROR_SUCCESS != ::RegSetKeyValueW(KeyApproved.Handle(), nullptr, szCLSID, REG_SZ, wszAppDisplayName, sizeof(__wchar_t) * ARRAYSIZE(wszAppDisplayName)))
		return E_ACCESSDENIED;

	#ifdef _MERGE_PROXYSTUB
	if (FAILED(hr))
		return hr;
	hr = PrxDllRegisterServer();
#endif
		return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();

	CLSID ClassID = { 0 };
	if (FAILED(::CLSIDFromProgID(&wszAppID[0], &ClassID)))
		return E_ACCESSDENIED;

	LPOLESTR lpszCLSID(nullptr);
	__wchar_t szCLSID[64] = { 0 };
	if (FAILED(::StringFromCLSID(ClassID, &lpszCLSID)))
		return E_ACCESSDENIED;
	else
	{
		::StringCchCopyW(szCLSID, 64, lpszCLSID);
		::CoTaskMemFree(lpszCLSID);
	}

	CXRegKey KeyApproved;
	HKEY hKeyApproved(nullptr); 

	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, wszRegKey, 0, KEY_SET_VALUE, &hKeyApproved))
		return E_ACCESSDENIED;
	else
		KeyApproved.Assign(hKeyApproved);

	if (ERROR_SUCCESS != ::RegDeleteValueW(KeyApproved.Handle(), wszRegKey))
		return E_ACCESSDENIED;

	#ifdef _MERGE_PROXYSTUB
	if (FAILED(hr))
		return hr;
	hr = PrxDllRegisterServer();
	if (FAILED(hr))
		return hr;
	hr = PrxDllUnregisterServer();
#endif
		return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{	
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}
