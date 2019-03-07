#include "stdafx.h"
#include <WindowsX.h>
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <strsafe.h>
#include <memory>

const __wchar_t lpwszRegistryDlgProp[] = L"RegistryDlgProp_ClassRef";

INT_PTR CALLBACK CPE32PropsView::ChildRegistryViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR iResult(0);
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{

			HWND hMainDlg(::GetParent(hDlg));
			__PEIMGRESOURCEITEM* pRegResItem(reinterpret_cast<__PEIMGRESOURCEITEM*>(lParam));
			HWND hTab(::GetDlgItem(hMainDlg, IDC_EDIT_REGISTRY));
			HWND hEdit(::GetDlgItem(hDlg, IDC_EDIT_REGISTRY));

			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(::GetParent(hDlg), lpwszRegistryDlgProp)));

			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTab, VSCLASS_EXPLORER, nullptr);
				::EnableThemeDialogTexture(hTab, ETDT_ENABLEAEROWIZARDTAB);
				::SetWindowTheme(hEdit, VSCLASS_EDIT, nullptr);
			}

			::MoveWindow(hDlg, pView ->mf_rcRegDisplayRect.left, pView ->mf_rcRegDisplayRect.top, pView ->mf_rcRegDisplayRect.right - pView ->mf_rcRegDisplayRect.left, pView ->mf_rcRegDisplayRect.bottom - pView ->mf_rcRegDisplayRect.top, FALSE);
	
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hEdit, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);

			char* pszResReg(reinterpret_cast<char*>(pRegResItem ->lpResItemDataPtr));
			int cniBufferLen(0);

			if (0 != (cniBufferLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszResReg, static_cast<int>(pRegResItem ->dwResItemDataSize), nullptr, 0)))
			{
				auto Deleter1 = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
				std::unique_ptr<__wchar_t, decltype(Deleter1)> pszTmp(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) * (cniBufferLen + 1))), Deleter1);

				if (pszTmp)
				{
					::ZeroMemory(pszTmp.get(), (cniBufferLen + 1) * sizeof(__wchar_t));
					if (0 != ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszResReg, static_cast<int>(pRegResItem ->dwResItemDataSize), pszTmp.get(), cniBufferLen))
					{
						int nSelPos(::GetWindowTextLengthW(hEdit));
						if (!nSelPos)
							::SetWindowTextW(hEdit, pszTmp.get());
						else
						{
							::SendMessageW(hEdit, EM_SETSEL, nSelPos, nSelPos);
							::SendMessageW(hEdit, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(pszTmp.get()));
						}
					}
				}
				iResult = 1;
			}

			::SetWindowPos(hDlg, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			::ShowWindow(hDlg, SW_SHOWNORMAL);
		}
	}
	return iResult;
}

bool CALLBACK CPE32PropsView::EnumRegistryRes(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);

	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;
		HWND hTab(pRTEFC ->hTree);

		__wchar_t wszLCIDName[LOCALE_NAME_MAX_LENGTH + 1] = { 0 };
		if (!::VerLanguageNameW(pResItemDesc ->lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH))
			if (!::LCIDToLocaleName(pResItemDesc ->lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, 0))
				::StringCchPrintfW(&wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, L"%4X", pResItemDesc ->lcid);

		__wchar_t ResourceName[2 * MAX_PATH] = { 0 };

		if (rtByString == pResItemDesc ->ResIDType)
			::StringCchPrintfW(ResourceName, 2 * MAX_PATH, L"[%s] %s", wszLCIDName, pResItemDesc ->pwszName);
		else
			::StringCchPrintfW(ResourceName, 2 * MAX_PATH, L"[%s] #%d", wszLCIDName, pResItemDesc ->dwID);

		TCITEMW tie = { 0 };
		tie.mask = TCIF_TEXT | TCIF_IMAGE; 
		tie.iImage = -1;
		tie.pszText = ResourceName;
		::StringCchLengthW(ResourceName, 2 * MAX_PATH, reinterpret_cast<size_t*>(&tie.cchTextMax));

		::DSA_InsertItem(pRTEFC ->pMainDlg ->mf_RegistryUnits.mf_HDSA, DSA_APPEND, pResItemDesc);

		TabCtrl_InsertItem(hTab, TabCtrl_GetItemCount(hTab), &tie);
	}
	return fbResult;
}

HWND CPE32PropsView::CreateChildRegistryDlg(__in HWND hDlg, __in HWND hTab, __in UINT nDlgID)
{
	HWND hRegResViewDlg(nullptr);
	if (hDlg && ::IsWindow(hDlg))
	{
		CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszRegistryDlgProp)));
		if (pView)
		{
			int nCount(DSA_GetItemCount(pView ->mf_RegistryUnits.mf_HDSA));

			if (static_cast<UINT>(nCount) > nDlgID)
			{
				__PEIMGRESOURCEITEM* pRegResItem(reinterpret_cast<__PEIMGRESOURCEITEM*>(::DSA_GetItemPtr(pView ->mf_RegistryUnits.mf_HDSA, static_cast<UINT_PTR>(nDlgID))));
				if (pRegResItem)
					hRegResViewDlg = ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_REGISTRYUNIT)), hDlg, ChildRegistryViewDlgInitProc, reinterpret_cast<LPARAM>(pRegResItem));
			}
		}
	}
	return hRegResViewDlg;
}

INT_PTR CALLBACK CPE32PropsView::ChildRegistryDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTab(::GetDlgItem(hDlg, IDC_TAB_REGISTRY));

			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTab, VSCLASS_EXPLORER, nullptr);
				::EnableThemeDialogTexture(hTab, ETDT_ENABLEAEROWIZARDTAB);

			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.right;
			::MoveWindow(hTab, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);

			RES_TYPE_ENUM_CONTEXT rtec;
 			rtec.dwStructSize = sizeof(RES_TYPE_ENUM_CONTEXT);
			rtec.pMainDlg = pView;
			rtec.hDlg = hDlg;
			rtec.hTree = hTab;
			rtec.pfnEnumResByType = EnumRegistryRes;
			
			//GetPEImgRegistryResTypeID
			DWORD dwRegistryRegTypeID;
			if (pView ->mf_pPE32 ->GetPEImgRegistryResTypeID(&dwRegistryRegTypeID))
				pView ->mf_pPE32 ->EnumPEImgResourcesByType(dwRegistryRegTypeID, dynamic_cast<IPE32ResourcesCallback*>(pView), &rtec);
			
			pView ->mf_hwndChildTypeLib = nullptr;
			TabCtrl_SetPadding(hTab, 2 * ::GetSystemMetrics(SM_CXDLGFRAME), 2 * ::GetSystemMetrics(SM_CYDLGFRAME));
			TabCtrl_SetCurSel(hTab, 0);

			if (!::SetPropW(hDlg, lpwszRegistryDlgProp, pView))
				return 0;
			else
			{
				//	Adjust Tab view control
				RECT rcTab, rcDisplay;
				::SetRectEmpty(&rcTab);
				::SetRectEmpty(&rcDisplay);
				::GetWindowRect(hTab, &rcTab);
				::MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)(&rcTab), (sizeof(RECT) / sizeof(POINT)));
				::OffsetRect(&rcTab, - rcTab.left, - rcTab.top);
				::CopyRect(&rcDisplay, &rcTab); 
				TabCtrl_AdjustRect(hTab, FALSE, &rcDisplay);
				::MapWindowPoints(hTab, hDlg, (LPPOINT)(&rcDisplay), (sizeof(RECT) / sizeof(POINT)));
				::CopyRect(&pView ->mf_rcRegDisplayRect, &rcDisplay);

				pView ->mf_hwndChildRegistry = pView ->CreateChildRegistryDlg(hDlg, hTab, 0);
				TabCtrl_SetCurSel(hTab, 0);
				if (pView ->mf_hwndChildTypeLib)
				{
					::SetWindowLongW(pView ->mf_hwndChildRegistry, GWL_STYLE, ::GetWindowLongW(pView ->mf_hwndChildRegistry, GWL_STYLE) | WS_VISIBLE);
					::SetWindowPos(pView ->mf_hwndChildRegistry, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER  | SWP_FRAMECHANGED);
					::SetWindowPos(pView ->mf_hwndChildRegistry, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE);
					::ShowWindow(hDlg, SW_SHOWNORMAL);
				}
			}
		}
		return 1;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh(reinterpret_cast<LPNMHDR>(lParam));
			if (pnmh ->idFrom == IDC_TAB_REGISTRY)
			{
				switch(pnmh ->code)
				{
				case TCN_SELCHANGE:
					{
						int iSel(TabCtrl_GetCurSel(::GetDlgItem(hDlg, pnmh ->idFrom)));
						CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszRegistryDlgProp)));
						if (pView)
						{
							if (pView ->mf_hwndChildRegistry)
							{
								::DestroyWindow(pView ->mf_hwndChildRegistry);
								pView ->mf_hwndChildRegistry = nullptr;
							}

							pView ->mf_hwndChildRegistry = pView ->CreateChildRegistryDlg(/*::GetDlgItem(hDlg, pnmh ->idFrom)*/hDlg, pnmh ->hwndFrom, static_cast<UINT>(iSel));
							if (pView ->mf_hwndChildRegistry)
							{
								::SetWindowLongW(pView ->mf_hwndChildRegistry, GWL_STYLE, ::GetWindowLongW(pView ->mf_hwndChildRegistry, GWL_STYLE) | WS_VISIBLE);
								::SetWindowPos(pView ->mf_hwndChildRegistry, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
								::SetWindowPos(pView ->mf_hwndChildRegistry, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
								::ShowWindow(pView ->mf_hwndChildTypeLib, SW_SHOW);
							}
						}
					}
					return 1;
				}
			}
		}
		break;
	case WM_DESTROY:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszRegistryDlgProp)));
			if (pView)
			{
				if (pView ->mf_hwndChildTypeLib)
				{
					::DestroyWindow(pView ->mf_hwndChildRegistry);
					pView ->mf_hwndChildRegistry = nullptr;
				}

				::RemovePropW(hDlg, lpwszRegistryDlgProp);
				int nCount(DSA_GetItemCount(pView ->mf_RegistryUnits.mf_HDSA));
				if (nCount)
					::DSA_DeleteAllItems(pView ->mf_RegistryUnits.mf_HDSA);
			}
		}
		break;
	}
	return 0;
}

