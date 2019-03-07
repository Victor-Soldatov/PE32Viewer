#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#define STRSAFE_LIB
#include <strsafe.h>
#include <time.h>

#include <stdarg.h>         // for va_start, etc.
#include <memory>

typedef struct __tagIMPORT_ENUM_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HTREEITEM htiImport;
}
IMPORT_ENUM_CONTEXT, *PIMPORT_ENUM_CONTEXT;

INT_PTR CALLBACK CPE32PropsView::ChildImportDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTree(::GetDlgItem(hDlg, IDC_IMPORT_TREE));
			if (::IsThemeActive())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			}

			PIMAGE_DATA_DIRECTORY pImgDataDir(nullptr);
			pView ->mf_pPE32 ->GetDataDirPtr(&pImgDataDir);

			HTREEITEM htiImport(TVI_ROOT);
			HTREEITEM htiDelayImport(TVI_ROOT);

			if ((pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) && (pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress))
			{
				//	Import & delayed import
				CResString Import(IDS_IMPORT);
				htiImport = AddItemToTreeView(hTree, nullptr, TVI_ROOT, Import.m_bszResString);

				CResString DelayImport(IDS_DELAY_IMPORT);
				htiDelayImport = AddItemToTreeView(hTree, nullptr, TVI_ROOT, DelayImport.m_bszResString);
			}

			if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
			{
				IMPORT_ENUM_CONTEXT iec;

				iec.dwStructSize = sizeof(IMPORT_ENUM_CONTEXT);
				iec.pMainDlg = pView;
				iec.hDlg = hDlg;
				iec.htiImport = htiImport;

				int nTotal(0);

				pView ->mf_pPE32 ->EnumPEImgImports(dynamic_cast<IPE32ImportsCallback*>(pView), &iec, &nTotal);
			}

			if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress)
			{
				IMPORT_ENUM_CONTEXT iec;

				iec.dwStructSize = sizeof(IMPORT_ENUM_CONTEXT);
				iec.pMainDlg = pView;
				iec.hDlg = hDlg;
				iec.htiImport = htiDelayImport;

				int nTotal(0);

				pView ->mf_pPE32 ->EnumPEImgDelayImports(dynamic_cast<IPE32ImportsCallback*>(pView), &iec, &nTotal);
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hTree, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
		}
		return 1;
	}
	return 0;
}

PIMPORTMODULE CPE32PropsView::FindImportModuleStruct(__in HDSA hDSA, __in __wchar_t* lpcwszModuleImgName)
{
	PIMPORTMODULE lpIM(nullptr);
	if (hDSA)
	{
		int nCount;

		if (0 < (nCount = DSA_GetItemCount(hDSA)))
		{
			for (int nIndex(0); nIndex < nCount; ++nIndex)
			{
				PIMPORTMODULE lpIMTmp(reinterpret_cast<PIMPORTMODULE>(::DSA_GetItemPtr(hDSA, nIndex)));
				if (lpIMTmp && (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, lpcwszModuleImgName, -1, &lpIMTmp ->wsModuleName[0], -1)))
				{
					lpIM = lpIMTmp;
					break;
				}
			}
		}
	}
	return (lpIM);
}

PIMPORTMODULE CPE32PropsView::AddImportModuleStruct(__in HDSA hDSA, __in HTREEITEM hTreeItem,__in __wchar_t* lpcwszModuleImgName)
{
	PIMPORTMODULE lpIM(nullptr);
	if (lpcwszModuleImgName && hDSA)
	{
		auto Deleter = [](PIMPORTMODULE pim) { if (pim) ::CoTaskMemFree(pim); };
		std::unique_ptr<IMPORTMODULE, decltype(Deleter)> pIM(reinterpret_cast<IMPORTMODULE*>(::CoTaskMemAlloc(sizeof(IMPORTMODULE))), Deleter);

		if (pIM)
		{
			::ZeroMemory(pIM.get(), sizeof(IMPORTMODULE));
			pIM ->hTreeItem = hTreeItem;
			::StringCchCopyW(&pIM ->wsModuleName[0], MAX_PATH, lpcwszModuleImgName);
			DSA_AppendItem(hDSA, pIM.get());
			lpIM = pIM.release();
		}
	}
	return lpIM;
}

STDMETHODIMP CPE32PropsView::EnumPE32ImportSymbols(__in BSTR pwszModuleName, __in __PEIMGIMPORTITEM* pItemDesc, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PIMPORT_ENUM_CONTEXT pIEC(reinterpret_cast<PIMPORT_ENUM_CONTEXT>(pContext));
	if (pIEC && sizeof(IMPORT_ENUM_CONTEXT) == pIEC ->dwStructSize)
	{
		hResult = S_OK;
		PIMPORTMODULE pIM(FindImportModuleStruct(pIEC ->pMainDlg ->mf_ImportModules.mf_HDSA, pwszModuleName));
		if (pIM)
		{
				//	Module item is found.
			if (ikByOrdinal == pItemDesc ->ImportKind)
				{
					__wchar_t wszOrdinal[MAX_PATH + 1] = { 0 };
					if (SUCCEEDED(::StringCchPrintfW(wszOrdinal, MAX_PATH, L"#%u", pItemDesc ->dwOrdinal)))
					{
						CComBSTR Ordinal = wszOrdinal;
						AddItemToTreeView(::GetDlgItem(pIEC ->hDlg, IDC_IMPORT_TREE), pIM ->hTreeItem, TVI_SORT, Ordinal);
					}
				}
				else
					AddItemToTreeViewA(::GetDlgItem(pIEC ->hDlg, IDC_IMPORT_TREE), pIM ->hTreeItem, TVI_FIRST, reinterpret_cast<char*>(pItemDesc ->pAnsiName));
		}
		else
		{
			CComBSTR ModuleName = pwszModuleName;
			HTREEITEM hTreeItem(AddItemToTreeView(::GetDlgItem(pIEC ->hDlg, IDC_IMPORT_TREE), pIEC ->htiImport, TVI_SORT, ModuleName));
			if (hTreeItem)
			{
				pIM = AddImportModuleStruct(pIEC ->pMainDlg ->mf_ImportModules.mf_HDSA, hTreeItem, pwszModuleName);
				if (ikByOrdinal == pItemDesc ->ImportKind)
				{
					__wchar_t wszOrdinal[MAX_PATH + 1] = { 0 };
					if (SUCCEEDED(::StringCchPrintfW(wszOrdinal, MAX_PATH, L"#%u", pItemDesc ->dwOrdinal)))
					{
						CComBSTR Ordinal = wszOrdinal;
						AddItemToTreeView(::GetDlgItem(pIEC ->hDlg, IDC_IMPORT_TREE), hTreeItem, TVI_SORT, Ordinal);
					}
				}
				else
					AddItemToTreeViewA(::GetDlgItem(pIEC ->hDlg, IDC_IMPORT_TREE), hTreeItem, TVI_SORT, reinterpret_cast<char*>(pItemDesc ->pAnsiName));
			}

		}
	}
	if (pwszModuleName)
		::SysFreeString(pwszModuleName);
	return hResult;
}

ULONG STDMETHODCALLTYPE CPE32PropsView::AddRef(void)
{
	++mf_nImportCount;
	return mf_nImportCount;
}

ULONG STDMETHODCALLTYPE CPE32PropsView::Release(void)
{
	if (mf_nImportCount)
		--mf_nImportCount;
	return mf_nImportCount;
}

STDMETHODIMP CPE32PropsView::QueryInterface(REFIID riid, void** ppvObj)
{
	return E_NOINTERFACE;
}

