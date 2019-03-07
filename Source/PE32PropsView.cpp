// PE32PropsView.cpp : Implementation of CPE32PropsView

#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#define STRSAFE_LIB
#include <strsafe.h>
#include <time.h>

#include <stdarg.h>         // for va_start, etc.

UINT arrDataDirEntry[] = {
	IDS_NT_OPT_DATA_DIR_EXPORT,
	IDS_NT_OPT_DATA_DIR_IMPORT,
	IDS_NT_OPT_DATA_DIR_RESOURCE,
	IDS_NT_OPT_DATA_DIR_EXCEPTION,
	IDS_NT_OPT_DATA_DIR_SECURITY,
	IDS_NT_OPT_DATA_DIR_BASERELOC,
	IDS_NT_OPT_DATA_DIR_DEBUG,
	IDS_NT_OPT_DATA_DIR_ARCHITECTURE,
	IDS_NT_OPT_DATA_DIR_GLOBALPTR,
	IDS_NT_OPT_DATA_DIR_TLS,
	IDS_NT_OPT_DATA_DIR_LOAD_CONFIG,
	IDS_NT_OPT_DATA_DIR_BOUND_IMPORT,
	IDS_NT_OPT_DATA_DIR_IAT,
	IDS_NT_OPT_DATA_DIR_DELAY_IMPORT,
	IDS_NT_OPT_DATA_DIR_COM_DESCRIPTOR
};

DLGTEMPLATE* CPE32PropsView::LockDlgRes(__in __wchar_t* lpwszResName)
{ 
	HRSRC hrsrc(::FindResourceW(hDLLInstance, lpwszResName, RT_DIALOG)); 
	HGLOBAL hGlobalRes;
	if (hrsrc)
		hGlobalRes = ::LoadResource(hDLLInstance, hrsrc); 
	else
		return nullptr;
	if (hGlobalRes)
	{
		DLGTEMPLATE* lpTemplate(reinterpret_cast<DLGTEMPLATE*>(::LockResource(hGlobalRes)));
		::FreeResource(hGlobalRes);
		return (lpTemplate); 
	}
	else 
		return nullptr;
}

void MakeReportView(HWND hListView, LPCWSTR* lpwTitles, const int iTitlesCount)
{
	RECT rc = { 0 };
	DWORD dwStyle(static_cast<DWORD>(::GetWindowLongW(hListView, GWL_STYLE)));
	::SetWindowLongW(hListView, GWL_STYLE, dwStyle | LVS_REPORT);
	::GetClientRect(hListView, &rc);

	for (int iIndex(0); iIndex < iTitlesCount; ++iIndex)
	{
		LVCOLUMNW lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvc.pszText = const_cast<LPWSTR>(lpwTitles[iIndex]);
		lvc.cx = (rc.right - rc.left) / iTitlesCount;
		if (iIndex % 2)
			lvc.fmt = LVCFMT_LEFT;
		else
			lvc.fmt = LVCFMT_RIGHT;
		ListView_InsertColumn(hListView, iIndex, &lvc);
	}
}

void AddStringToReportView(HWND hListView, LPCWSTR* lpwszStr, const int nColumnsCount)
{
	int nCount;
	if (::IsWindowUnicode(hListView))
		nCount = ::SendMessageW(hListView, LVM_GETITEMCOUNT, 0, 0);
	else
		nCount = ::SendMessageA(hListView, LVM_GETITEMCOUNT, 0, 0);

	LVITEMW lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = nCount;
	lvi.pszText = const_cast<__wchar_t*>(lpwszStr[0]);
	lvi.cchTextMax = ::lstrlenW(lpwszStr[0]);
	int nStringNo = ListView_InsertItem(hListView, &lvi);

	//	Fill other columns
	for (int iIndex(1); iIndex < nColumnsCount; ++iIndex)
		ListView_SetItemText(hListView, nStringNo, iIndex, const_cast<LPWSTR>(lpwszStr[iIndex]));
}

LRESULT CPE32PropsView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CPE32PropsView>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	if (::IsThemeActive())
	{
		if ((STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT) != (::GetThemeAppProperties() & (STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT)))
		{
			DWORD dwFlags(STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);
			::SetThemeAppProperties(dwFlags);
		}

		::SetWindowTheme(m_hWnd, VSCLASS_AEROWIZARD, nullptr);
		::EnableThemeDialogTexture(m_hWnd, ETDT_ENABLE);

		::SetWindowTheme(::GetDlgItem(m_hWnd, IDC_TAB), VSCLASS_EXPLORER, nullptr);

		::EnableThemeDialogTexture(::GetDlgItem(m_hWnd, IDC_TAB), ETDT_ENABLEAEROWIZARDTAB);
		::SetWindowTheme(::GetDlgItem(m_hWnd, IDOK), VSCLASS_BUTTON, nullptr);
	}

	mf_hDlgSmIcon = static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCE(IDI_DLGLOGOICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
	if (!mf_hDlgSmIcon)
		mf_hDlgSmIcon = ::LoadIcon(nullptr, IDI_ASTERISK);

	mf_hDlgIcon = static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCE(IDI_DLGLOGOICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
	if (!mf_hDlgIcon)
		mf_hDlgIcon = ::LoadIcon(nullptr, IDI_ASTERISK);

	::SendMessageW(m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(mf_hDlgSmIcon));
	::SendMessageW(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(mf_hDlgIcon));

	__wchar_t szFileName[MAX_PATH + 1] = { 0 };
	CComBSTR FileName;
	
	mf_pPE32 ->GetPE32FileName(&FileName.m_str);
	::StringCchCopyW(szFileName, ARRAYSIZE(szFileName), FileName.m_str);
	//::lstrcpynW(szFileName, FileName.m_str, MAX_PATH);

	CResString HdrText(IDS_DLGTEXT);
	if (HdrText.Length())
	{
		::StringCchCatW(szFileName, MAX_PATH, HdrText.m_bszResString.m_str);
		HdrText.m_bszResString.Empty();
	}

	::SetWindowTextW(m_hWnd, szFileName);	
	::SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	TRACKMOUSEEVENT tme = { 0 };
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.hwndTrack = m_hWnd;
	tme.dwHoverTime = HOVER_DEFAULT;

	::TrackMouseEvent(&tme);

	HWND hwndOwner;
	HWND hButton(::GetDlgItem(m_hWnd, IDOK));
	RECT rcDlg, rcOwner;

	if ((hwndOwner = ::GetParent(m_hWnd)) == nullptr) 
		hwndOwner = ::GetDesktopWindow(); 

	::GetWindowRect(hwndOwner, &rcOwner); 
	::GetWindowRect(m_hWnd, &rcDlg); 

	LONG nWidth(rcDlg.right - rcDlg.left);
	LONG nHeight(rcDlg.bottom - rcDlg.top);
	LONG nOffsetX(mf_ptInvokePt.x - rcDlg.left);
	LONG nOffsetY(mf_ptInvokePt.y - rcDlg.top);
	::OffsetRect(&rcDlg, nOffsetX, nOffsetY);

	if (rcDlg.right > rcOwner.right)
		::OffsetRect(&rcDlg, -nWidth, 0);
	if (rcDlg.bottom > rcOwner.bottom)
		::OffsetRect(&rcDlg, 0, -nOffsetY);

	::SetWindowPos(m_hWnd, HWND_TOP, rcDlg.left, rcDlg.top, rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top, SWP_NOZORDER | SWP_FRAMECHANGED);
	HWND hwndTab(::GetDlgItem(m_hWnd, IDC_TAB));

	TabCtrl_SetPadding(hwndTab, 2 * ::GetSystemMetrics(SM_CXDLGFRAME), ::GetSystemMetrics(SM_CYDLGFRAME));

	RECT rcButton;

	::GetWindowRect(hwndTab, &rcDlg); 
	::GetClientRect(m_hWnd, &rcOwner);
	::GetWindowRect(hButton, &rcButton); 
	::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)(&rcButton), (sizeof(RECT) / sizeof(POINT)));
	::OffsetRect(&rcButton, -rcButton.left, -rcButton.top);
	::MoveWindow(hButton, rcOwner.right - rcButton.right - 2 * ::GetSystemMetrics(SM_CXDLGFRAME), rcOwner.bottom - rcButton.bottom - 2 * ::GetSystemMetrics(SM_CYDLGFRAME), rcButton.right - rcButton.left, rcButton.bottom - rcButton.top, FALSE);

	::GetWindowRect(hButton, &rcButton); 
	::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)(&rcButton), (sizeof(RECT) / sizeof(POINT)));

	::OffsetRect(&rcOwner, -rcOwner.left, -rcOwner.top);
	::MapWindowPoints(hwndTab, m_hWnd, (LPPOINT)(&rcDlg), (sizeof(RECT) / sizeof(POINT)));
	::OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
	::SetWindowPos(hwndTab, HWND_TOP, ::GetSystemMetrics(SM_CXDLGFRAME), ::GetSystemMetrics(SM_CYDLGFRAME), rcOwner.right - ::GetSystemMetrics(SM_CXDLGFRAME) - 1, rcButton.top - 2 * ::GetSystemMetrics(SM_CYDLGFRAME), SWP_NOZORDER);

	int nTabIndex(0);
	TCITEMW tie = { 0 };
	tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
	tie.iImage = -1;
	//	Headers tab
	tie.lParam = tiHeaders;	

	DLG_TAB_ITEM dti;
	dti.dwStructSize = sizeof(DLG_TAB_ITEM);
	dti.hChildDlg = nullptr;

	CResString TabHdrText(IDS_HEADER);
	tie.pszText = TabHdrText.m_bszResString;
	TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

	dti.tiTabID = tiHeaders;
	dti.dwResID = IDD_DLGHEADER;

	if (mf_DlgDescriptors.mf_HDSA)
		::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);

	//	Sections tab
	tie.lParam = tiSections;	

	CResString TabSectionsText(IDS_SECTIONS);
	tie.pszText = TabSectionsText.m_bszResString;
	TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

	dti.tiTabID = tiSections;
	dti.dwResID = IDD_DLG_SECTIONS;

	if (mf_DlgDescriptors.mf_HDSA)
		::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);

	PIMAGE_DATA_DIRECTORY pImgDataDir(nullptr);
	mf_pPE32 ->GetDataDirPtr(&pImgDataDir);

	//	Import tab
	if ((pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress) || (pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress))
	{
		tie.lParam = tiImport;	

		CResString ImportText(IDS_TAB_IMPORT);
		tie.pszText = ImportText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiImport;
		dti.dwResID = IDD_DLG_IMPORT;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	Export tab
	if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
	{
		tie.lParam = tiExport;	

		CResString ExportText(IDS_TAB_EXPORT);
		tie.pszText = ExportText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiExport;
		dti.dwResID = IDD_DLG_EXPORT;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	Security tab
	if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_SECURITY].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress && S_OK == mf_pPE32 ->IsPEImgFileDigitallySigned())
	{
		tie.lParam = tiSecurity;	

		CResString SecurityText(IDS_TAB_SECURITY);
		tie.pszText = SecurityText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiSecurity;
		dti.dwResID = IDD_DLG_SECURITY;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	Resources tab
	if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		tie.lParam = tiResources;	

		CResString ResourcesText(IDS_TAB_RESOURCES);
		tie.pszText = ResourcesText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiResources;
		dti.dwResID = IDD_DLG_RESOURCES;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	Type Libs tab
	if (S_OK == mf_pPE32 ->IsPEImgContainsTypeLib())
	{
		tie.lParam = tiTypeLib;	

		CResString ResourcesText(IDS_RES_TYPE_TYPELIB);
		tie.pszText = ResourcesText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiTypeLib;
		dti.dwResID = IDD_DLG_TYPELIB;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	REGISTRY
	if (S_OK == mf_pPE32 ->IsPEImgContainsRegistryRes())
	{
		tie.lParam = tiRegistry;	

		CResString ResourcesText(IDS_RES_TYPE_REGISTRY);
		tie.pszText = ResourcesText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);
		
		dti.tiTabID = tiRegistry;
		dti.dwResID = IDD_DLG_REGISTRY;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	Manifest
	if (S_OK == mf_pPE32 ->IsPEImgContainsManifest())
	{
		tie.lParam = tiManifest;	

		CResString ResourcesText(IDS_RES_TYPE_MANIFEST);
		tie.pszText = ResourcesText.m_bszResString;
		TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

		dti.tiTabID = tiManifest;
		dti.dwResID = IDD_DLG_MANIFEST;

		if (mf_DlgDescriptors.mf_HDSA)
			::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);
	}

	//	System tab
	tie.lParam = tiSystem;	

	CResString TabSystemText(IDS_SYSTEM);
	tie.pszText = TabSystemText.m_bszResString;
	TabCtrl_InsertItem(GetDlgItem(IDC_TAB), nTabIndex++, &tie);

	dti.tiTabID = tiSystem;
	dti.dwResID = IDD_DLGSYSTEM;

	if (mf_DlgDescriptors.mf_HDSA)
		::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);

	//	About tab
	tie.lParam = tiAbout;	

	CResString TabAboutText(IDS_ABOUT);
	tie.pszText = TabAboutText.m_bszResString;	
	TabCtrl_SetCurSel(GetDlgItem(IDC_TAB), TabCtrl_InsertItem(GetDlgItem(IDC_TAB),  nTabIndex++, &tie));

	dti.tiTabID = tiAbout;
	dti.dwResID = IDD_DLGABOUT;

	if (mf_DlgDescriptors.mf_HDSA)
		::DSA_InsertItem(mf_DlgDescriptors.mf_HDSA, DSA_APPEND, &dti);

	//	Adjust Tab view control
	RECT rcTab;
	::SetRectEmpty(&rcTab);
	::GetWindowRect(hwndTab, &rcTab);
	::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)(&rcTab), (sizeof(RECT) / sizeof(POINT)));
	::OffsetRect(&rcTab, - rcTab.left, - rcTab.top);
	::CopyRect(&mf_rcDisplay, &rcTab); 
    TabCtrl_AdjustRect(hwndTab, FALSE, &mf_rcDisplay);
	::MapWindowPoints(hwndTab, m_hWnd, (LPPOINT)(&mf_rcDisplay), (sizeof(RECT) / sizeof(POINT)));

	mf_hChildDlg = nullptr;

	::SetWindowPos(hwndTab, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE);
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	TabCtrl_SetCurSel(hwndTab, 0);
	OnSelChanged(m_hWnd);

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

HTREEITEM CPE32PropsView::AddItemToTreeView(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in CComBSTR& lpwszInfoStr, __in LPARAM lParam)
{
	HTREEITEM hTVItem(nullptr);
	if (hTreeViewCtrl && ::IsWindow(hTreeViewCtrl) && ::IsWindowUnicode(hTreeViewCtrl))
	{
		TVINSERTSTRUCTW tv_ins = { 0 };
		tv_ins.hParent = hParent;
		tv_ins.hInsertAfter = hInsertAfter;

		if (GetCommCtl32DllVersion() >= PACKVERSION(4,71))
		{
			TVITEMEXW tvi = { 0 };
			tvi.mask = TVIF_TEXT;
			if (lParam)
			{
				tvi.mask |= TVIF_PARAM;
				tvi.lParam = lParam;
			}
			tvi.pszText = lpwszInfoStr.m_str;
			tvi.cchTextMax = lpwszInfoStr.Length();

			::CopyMemory(&tv_ins.itemex, &tvi, sizeof(TVITEMEXW));
		}
		else
		{
			TVITEMW tvi = { 0 }; 

			tvi.mask = TVIF_TEXT;
			if (lParam)
			{
				tvi.mask |= TVIF_PARAM;
				tvi.lParam = lParam;
			}

			tvi.pszText = lpwszInfoStr.m_str;
			tvi.cchTextMax = lpwszInfoStr.Length();

			::CopyMemory(&tv_ins.item, &tvi, sizeof(TVITEMW));
		}

		hTVItem = TreeView_InsertItem(hTreeViewCtrl, &tv_ins);
	}
	return hTVItem;
}

HTREEITEM CPE32PropsView::AddBoldItemToTreeViewW(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in __wchar_t* lpwszInfoStr, __in const LPARAM lParam)
{
	HTREEITEM hTVItem(nullptr);
	if (hTreeViewCtrl && ::IsWindow(hTreeViewCtrl) && ::IsWindowUnicode(hTreeViewCtrl))
	{
		TVINSERTSTRUCTW tv_ins = { 0 };
		tv_ins.hParent = hParent;
		tv_ins.hInsertAfter = hInsertAfter;

		if (GetCommCtl32DllVersion() >= PACKVERSION(4,71))
		{
			TVITEMEXW tvi = { 0 };
			tvi.mask = TVIF_TEXT | TVIF_STATE;
			tvi.state = TVIS_BOLD;

			if (lParam)
			{
				tvi.mask |= TVIF_PARAM;
				tvi.lParam = lParam;
			}

			tvi.pszText = lpwszInfoStr;
			::StringCchLengthW(lpwszInfoStr, STRSAFE_MAX_CCH, reinterpret_cast<size_t*>(&tvi.cchTextMax));
			::CopyMemory(&tv_ins.itemex, &tvi, sizeof(TVITEMEXW));
		}
		else
		{
			TVITEMW tvi = { 0 }; 

			tvi.mask = TVIF_TEXT | TVIF_STATE;
			tvi.state = TVIS_BOLD;

			if (lParam)
			{
				tvi.mask |= TVIF_PARAM;
				tvi.lParam = lParam;
			}

			tvi.pszText = lpwszInfoStr;
			::StringCchLengthW(lpwszInfoStr, STRSAFE_MAX_CCH, reinterpret_cast<size_t*>(&tvi.cchTextMax));
			::CopyMemory(&tv_ins.item, &tvi, sizeof(TVITEMW));
		}

		hTVItem = TreeView_InsertItem(hTreeViewCtrl, &tv_ins);
	}
	return hTVItem;
}


HTREEITEM CPE32PropsView::AddItemToTreeViewA(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in char* lptszInfoStr)
{
	HTREEITEM hTVItem(nullptr);
	if (hTreeViewCtrl && ::IsWindow(hTreeViewCtrl) && ::IsWindowUnicode(hTreeViewCtrl))
	{
		__wchar_t wszSymbol[MAX_PATH + 1] = { 0 };

		TVINSERTSTRUCTW tv_ins = { 0 };
		tv_ins.hParent = hParent;
		tv_ins.hInsertAfter = hInsertAfter;

		if (GetCommCtl32DllVersion() >= PACKVERSION(4,71))
		{
			TVITEMEXW tvi = { 0 };
			tvi.mask = TVIF_TEXT;

			if (lptszInfoStr)
			{
				//	Try to get ansi string length (in chars)
				size_t dwcnChars(0);
				HRESULT hr(::StringCchLengthA((STRSAFE_PCNZCH)(lptszInfoStr), STRSAFE_MAX_CCH, &dwcnChars));
				if (SUCCEEDED(hr) && (dwcnChars < MAX_PATH))
				{
					UINT dwcnWChars(static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, lptszInfoStr, -1, nullptr, 0)));
					int cchTextMax(0);
					if ((dwcnWChars != 0) && (dwcnWChars < MAX_PATH) && (cchTextMax = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lptszInfoStr, -1, &wszSymbol[0], MAX_PATH)))
					{
						tvi.pszText = &wszSymbol[0];
						::StringCchLengthW(&wszSymbol[0], MAX_PATH + 1, reinterpret_cast<size_t*>(&tvi.cchTextMax));
						::CopyMemory(&tv_ins.itemex, &tvi, sizeof(TVITEMEXW));
					}
				}
			}			
		}
		else
		{
			TVITEMW tvi = { 0 }; 
			tvi.mask = TVIF_TEXT;

			__wchar_t wszSymbol[MAX_PATH + 1] = { 0 };
			if (lptszInfoStr)
			{
				//	Try to get ansi string length (in chars)
				size_t dwcnChars(0);
				HRESULT hr(::StringCchLengthA((STRSAFE_PCNZCH)(lptszInfoStr), STRSAFE_MAX_CCH, &dwcnChars));
				if (SUCCEEDED(hr) && (dwcnChars < MAX_PATH))
				{
					UINT dwcnWChars(static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, lptszInfoStr, -1, nullptr, 0)));
					if ((dwcnWChars != 0) && (dwcnWChars < MAX_PATH) && ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lptszInfoStr, -1, &wszSymbol[0], MAX_PATH))
					{
						tvi.pszText = &wszSymbol[0];
						::StringCchLengthW(&wszSymbol[0], MAX_PATH + 1, reinterpret_cast<size_t*>(&tvi.cchTextMax));
						::CopyMemory(&tv_ins.item, &tvi, sizeof(TVITEMW));
					}
				}
			}
		}

		hTVItem = TreeView_InsertItem(hTreeViewCtrl, &tv_ins);
	}
	return hTVItem;
}

void CPE32PropsView::FormatResource(__out CComBSTR& bstrResult, __in DWORD dwMsgID, ...)
{
	va_list argptr;
    va_start(argptr, dwMsgID);

	CResString FormatStr(dwMsgID);
	__wchar_t szBuffer[4096] = { 0 };

	if (FormatStr.Length() && SUCCEEDED(::StringCchVPrintfW(szBuffer, 4096, FormatStr.m_bszResString, argptr)))
		bstrResult = szBuffer;

	va_end(argptr);
}

INT_PTR CALLBACK CPE32PropsView::ChildHeaderDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTree(::GetDlgItem(hDlg, IDC_HEADERS_TREE));			

			if (::IsThemeActive())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			}

			PIMAGE_DOS_HEADER pDOSHdr;
			pView ->mf_pPE32 ->GetDOSHeaderPtr(&pDOSHdr);

			CResString DOSHeader(IDS_DOS_HEADER);
			HTREEITEM htiDOS(AddItemToTreeView(hTree, TVI_ROOT, TVI_ROOT, DOSHeader.m_bszResString));

			CComBSTR Line;
			FormatResource(Line, IDS_DOS_HEADER_MAGIC, pDOSHdr ->e_magic);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_LAST_PAGE, pDOSHdr ->e_cblp);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_PAGES_IN_FILE, pDOSHdr ->e_cp);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_RELOCATIONS, pDOSHdr ->e_crlc);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_CPARHDR, pDOSHdr ->e_cparhdr);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_MINALLOC, pDOSHdr ->e_minalloc);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_MAXALLOC, pDOSHdr ->e_maxalloc);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_SS, pDOSHdr ->e_ss);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_SP, pDOSHdr ->e_sp);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);
			
			FormatResource(Line, IDS_DOS_E_CSUM, pDOSHdr ->e_csum);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_IP, pDOSHdr ->e_ip);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_CS, pDOSHdr ->e_cs);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_LFARLC, pDOSHdr ->e_lfarlc);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_OVNO, pDOSHdr ->e_ovno);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			CResString E_Res(IDS_DOS_E_RES);
			HTREEITEM htiE_Res(AddItemToTreeView(hTree, htiDOS, TVI_LAST, E_Res.m_bszResString));

			for (int i(0); i < ARRAYSIZE(pDOSHdr ->e_res); ++i)
			{
				FormatResource(Line, IDS_DOS_E_RES_FMT, i, pDOSHdr ->e_res[i]);
				AddItemToTreeView(hTree, htiE_Res, TVI_LAST, Line);
			}

			FormatResource(Line, IDS_DOS_E_OEMID, pDOSHdr ->e_oemid);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			FormatResource(Line, IDS_DOS_E_OEMINFO, pDOSHdr ->e_oeminfo);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			CResString E_Res2(IDS_DOS_E_RES2);
			HTREEITEM htiE_Res2(AddItemToTreeView(hTree, htiDOS, TVI_LAST, E_Res2.m_bszResString));

			for (int i(0); i < ARRAYSIZE(pDOSHdr ->e_res2); ++i)
			{
				FormatResource(Line, IDS_DOS_E_RES2_FMT, i, pDOSHdr ->e_res2[i]);
				AddItemToTreeView(hTree, htiE_Res2, TVI_LAST, Line);
			}

			FormatResource(Line, IDS_DOS_E_LFANEW, pDOSHdr ->e_lfanew);
			AddItemToTreeView(hTree, htiDOS, TVI_LAST, Line);

			//	NT headers
			CResString NTHeaders(IDS_NT_HEADERS);
			HTREEITEM htiNTs(AddItemToTreeView(hTree, TVI_ROOT, htiDOS, NTHeaders.m_bszResString));

			PIMAGE_NT_HEADERS pImgNTHdrs(nullptr);
			pView ->mf_pPE32 ->GetNTHeadersPtr(&pImgNTHdrs);

			FormatResource(Line, IDS_NT_HEADERS_SIGN, pImgNTHdrs ->Signature);
			AddItemToTreeView(hTree, htiNTs, TVI_LAST, Line);

			//	File header
			PIMAGE_FILE_HEADER pImgFileHdr(nullptr);
			pView ->mf_pPE32 ->GetFileHeaderPtr(&pImgFileHdr);

			CResString NTFileHeader(IDS_NT_FILE_HEADER);
			HTREEITEM htiNTFile(AddItemToTreeView(hTree, htiNTs, TVI_LAST, NTFileHeader.m_bszResString));

			CResString MachineUnknown(IDS_NT_FH_MACHINE_UNKNOWN);
			switch(pImgFileHdr ->Machine)
			{
			case IMAGE_FILE_MACHINE_I386:
				MachineUnknown.m_bszResString = L"x86";
				break;
			case IMAGE_FILE_MACHINE_IA64:
				MachineUnknown.m_bszResString = L"Intel IPF";
				break;
			case IMAGE_FILE_MACHINE_AMD64:
				MachineUnknown.m_bszResString = L"x64";
				break;
			}

			FormatResource(Line, IDS_NT_FH_MACHINE, MachineUnknown.m_bszResString.m_str, pImgFileHdr ->Machine);
			AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_FH_NUM_SECTIONS, pImgFileHdr ->NumberOfSections);
			AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);

			struct tm Tm;
			if (!_localtime32_s(&Tm, reinterpret_cast<__time32_t*>(&pImgFileHdr ->TimeDateStamp)))
			{
				FormatResource(Line, IDS_DOS_E_LFANEW, Tm.tm_mday, Tm.tm_mon + 1, Tm.tm_year, Tm.tm_hour, Tm.tm_min, Tm.tm_sec);
				AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);
			}
			else
			{
				//	Unknown TimeDateStamp
				FormatResource(Line, IDS_NT_FH_TIMEDATESTAMP_UNK, pImgFileHdr ->TimeDateStamp);
				AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);
			}

			FormatResource(Line, IDS_NT_FH_SYMBOLTABLE_OFFSET, pImgFileHdr ->PointerToSymbolTable);
			AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_FH_SYMBOLTABLE_SIZE, pImgFileHdr ->NumberOfSymbols);
			AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_FH_OPT_HDR_SIZE, pImgFileHdr ->SizeOfOptionalHeader);
			AddItemToTreeView(hTree, htiNTFile, TVI_LAST, Line);

			if (pImgFileHdr ->Characteristics)
			{
				CResString NTFileHeaderChars(IDS_NT_FH_CHS);
				HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFile, TVI_LAST, NTFileHeader.m_bszResString));

				if (pImgFileHdr ->Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_RELOCS_STRIPPED);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_EXECUTABLE_IMAGE);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_LINE_NUMS_STRIPPED)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_LINE_NUMS_STRIPPED);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_LOCAL_SYMS_STRIPPED)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_LOCAL_SYMS_STRIPPED);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_LARGE_ADDRESS_AWARE);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_32BIT_MACHINE)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_32BIT_MACHINE);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_DEBUG_STRIPPED);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_NET_RUN_FROM_SWAP)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_NET_RUN_FROM_SWAP);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_SYSTEM)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_SYSTEM);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
				if (pImgFileHdr ->Characteristics & IMAGE_FILE_DLL)
				{
					CResString NTFileHeaderCharsLine(IDS_IMAGE_FILE_DLL);
					HTREEITEM htiNTFileChars(AddItemToTreeView(hTree, htiNTFileChars, TVI_LAST, NTFileHeaderCharsLine.m_bszResString));
				}
			}

			//	Optional header
			PIMAGE_OPTIONAL_HEADER pImgOptHdr(nullptr);
			pView ->mf_pPE32 ->GetOptionalHeaderPtr(&pImgOptHdr);

			CResString NTOptHeader(IDS_NT_OPT_HEADER);
			HTREEITEM htiNTOpt(AddItemToTreeView(hTree, htiNTs, TVI_LAST, NTOptHeader.m_bszResString));

			CResString ImgState(IDS_NT_OPT_MAGIC_STATE_UNKNOWN);

			switch(pImgOptHdr ->Magic)
			{
			case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
				ImgState.m_bszResString = L"IMAGE_NT_OPTIONAL_HDR32_MAGIC";
				break;
			case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
				ImgState.m_bszResString = L"IMAGE_NT_OPTIONAL_HDR64_MAGIC";
				break;
			case IMAGE_ROM_OPTIONAL_HDR_MAGIC:
				ImgState.m_bszResString = L"IMAGE_ROM_OPTIONAL_HDR_MAGIC";
				break;
			}

			FormatResource(Line, IDS_NT_OPT_MAGIC, pImgOptHdr ->Magic, ImgState.m_bszResString.m_str);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_LINKER_VERSION, pImgOptHdr ->MajorLinkerVersion, pImgOptHdr ->MinorLinkerVersion);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_CODE, pImgOptHdr ->SizeOfCode);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_INIT_DATA, pImgOptHdr ->SizeOfInitializedData);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_ENTRY_POINT, pImgOptHdr ->AddressOfEntryPoint);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_BASE_OF_CODE, pImgOptHdr ->BaseOfCode);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_BASE_OF_DATA, pImgOptHdr ->BaseOfData);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_IMAGE_BASE, pImgOptHdr ->ImageBase);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SECTION_ALIGN, pImgOptHdr ->SectionAlignment);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_FILE_ALIGN, pImgOptHdr ->FileAlignment);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_REQ_OS_VER, pImgOptHdr ->MajorOperatingSystemVersion, pImgOptHdr ->MinorOperatingSystemVersion);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_IMAGE_VER, pImgOptHdr ->MajorImageVersion, pImgOptHdr ->MinorImageVersion);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SUBSYSTEM_VER, pImgOptHdr ->MajorSubsystemVersion, pImgOptHdr ->MinorSubsystemVersion);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_IMAGE, pImgOptHdr ->SizeOfImage);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_HEADERS, pImgOptHdr ->SizeOfHeaders);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_CHECKSUM, pImgOptHdr ->CheckSum);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			UINT nSubsysID(IDS_NT_OPT_SUBSYSTEM_UNKNOWN);
			switch(pImgOptHdr ->Subsystem)
			{
			case IMAGE_SUBSYSTEM_UNKNOWN:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_UNKNOWN;
				break;
			case IMAGE_SUBSYSTEM_NATIVE:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_NATIVE;
				break;
			case IMAGE_SUBSYSTEM_WINDOWS_GUI:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_WINDOWS_GUI;
				break;
			case IMAGE_SUBSYSTEM_WINDOWS_CUI:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_WINDOWS_CUI;
				break;
			case IMAGE_SUBSYSTEM_OS2_CUI:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_OS2_CUI;
				break;
			case IMAGE_SUBSYSTEM_POSIX_CUI:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_POSIX_CUI;
				break;
			case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_NATIVE_WINDOWS;
				break;
			case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_WINDOWS_CE_GUI;
				break;
			case IMAGE_SUBSYSTEM_EFI_APPLICATION:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_EFI_APPLICATION;
				break;
			case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
				break;
			case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_EFI_RUNTIME_DRIVER;
				break;
			case IMAGE_SUBSYSTEM_EFI_ROM:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_EFI_ROM;
				break;
			case IMAGE_SUBSYSTEM_XBOX:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_XBOX;
				break;
			case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
				nSubsysID = IDS_NT_OPT_SUBSYSTEM_WINDOWS_BOOT_APPLICATION;
				break;
			}

			CResString Subsystem(nSubsysID);
			FormatResource(Line, IDS_NT_OPT_SUBSYSTEM, Subsystem.m_bszResString.m_str);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_DLL_CHARACTERISTICS, pImgOptHdr ->DllCharacteristics);
			HTREEITEM htiDLLChars(AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line));

			if (pImgOptHdr ->DllCharacteristics)
			{
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_DYNAMIC_BASE);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_FORCE_INTEGRITY);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NX_COMPAT)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_NX_COMPAT);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_ISOLATION)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_NO_ISOLATION);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_NO_SEH);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_BIND)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_NO_BIND);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_WDM_DRIVER)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_WDM_DRIVER);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
				if (pImgOptHdr ->DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)
				{
					CResString DllCharacteristics(IDS_NT_OPT_DLL_CHARS_TERMINAL_SERVER_AWARE);
					AddItemToTreeView(hTree, htiDLLChars, TVI_LAST, DllCharacteristics.m_bszResString);
				}
			}

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_STACK_RESERVE, pImgOptHdr ->SizeOfStackReserve);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_STACK_COMMIT, pImgOptHdr ->SizeOfStackCommit);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_HEAP_RESERVE, pImgOptHdr ->SizeOfHeapReserve);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_SIZE_OF_HEAP_COMMIT, pImgOptHdr ->SizeOfHeapCommit);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			FormatResource(Line, IDS_NT_OPT_NUMBER_OF_DDENTRIES, pImgOptHdr ->NumberOfRvaAndSizes);
			AddItemToTreeView(hTree, htiNTOpt, TVI_LAST, Line);

			if (pImgOptHdr ->NumberOfRvaAndSizes)
			{
				PIMAGE_DATA_DIRECTORY pImgDataDir;
				pView ->mf_pPE32 ->GetDataDirPtr(&pImgDataDir);

				CResString DataDir(IDS_NT_OPT_DATA_DIR);
				HTREEITEM htiDataDir(AddItemToTreeView(hTree, htiNTs, TVI_LAST, DataDir.m_bszResString));

				for (DWORD nDDIndex(0); nDDIndex < pImgOptHdr ->NumberOfRvaAndSizes; ++nDDIndex)
				{
					HTREEITEM htiDataDirEntry;
					if (nDDIndex < ARRAYSIZE(arrDataDirEntry))
					{
						CResString DataDirectoryEntry(arrDataDirEntry[nDDIndex]);
						if (pImgDataDir[nDDIndex].Size)
							htiDataDirEntry = AddBoldItemToTreeViewW(hTree, htiDataDir, TVI_LAST, DataDirectoryEntry.m_bszResString, 0);
						else
							htiDataDirEntry = AddItemToTreeView(hTree, htiDataDir, TVI_LAST, DataDirectoryEntry.m_bszResString);
					}
					else
					{
						FormatResource(Line, IDS_NT_OPT_DATA_DIR_ENTRY, nDDIndex + 1);
						if (pImgDataDir[nDDIndex].Size)
							htiDataDirEntry = AddBoldItemToTreeViewW(hTree, htiDataDir, TVI_LAST, Line, 0);
						else
							htiDataDirEntry = AddItemToTreeView(hTree, htiDataDir, TVI_LAST, Line);
					}

					FormatResource(Line, IDS_NT_OPT_DATA_DIR_VIRT_ADDRESS, pImgDataDir[nDDIndex].VirtualAddress);
					AddItemToTreeView(hTree, htiDataDirEntry, TVI_LAST, Line);
					FormatResource(Line, IDS_NT_OPT_DATA_DIR_SIZE, pImgDataDir[nDDIndex].Size);
					AddItemToTreeView(hTree, htiDataDirEntry, TVI_LAST, Line);
				}
			}			

			//	Adjust view
			if (::IsThemeActive())
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left/* - 2 * ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);

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

HWND CPE32PropsView::GetChildDlgHandle(__in int nIndex, __out_opt BOOL* pfbCreated)
{
	HWND hChildDlg(nullptr);
	int nCount;
	if (mf_DlgDescriptors.mf_HDSA && (nCount = DSA_GetItemCount(mf_DlgDescriptors.mf_HDSA)) && nIndex < nCount)
	{
		PDLG_TAB_ITEM pDTI;
		if (pDTI = reinterpret_cast<PDLG_TAB_ITEM>(::DSA_GetItemPtr(mf_DlgDescriptors.mf_HDSA, nIndex)))
		{
			if (!pDTI ->hChildDlg)
			{
				hChildDlg = pDTI ->hChildDlg = CreateChildDialog(pDTI ->dwResID);
				if (pfbCreated)
					*pfbCreated = TRUE;
			}
			else
			{
				hChildDlg = pDTI ->hChildDlg;
				if (pfbCreated)
					*pfbCreated = FALSE;
			}
		}
	}

	return hChildDlg;
}

DWORD CPE32PropsView::GetCommCtl32DllVersion(void)
{
    HMODULE hDll(::GetModuleHandleW(L"comctl32.dll"));
    DWORD dwVersion(0);

    if(hDll)
    {
        DLLGETVERSIONPROC pDllGetVersion(reinterpret_cast<DLLGETVERSIONPROC>(::GetProcAddress(hDll, "DllGetVersion")));
        if(pDllGetVersion)
        {
			DLLVERSIONINFO dvi = { 0 };
            dvi.cbSize = sizeof(dvi);

            if(SUCCEEDED((*pDllGetVersion)(&dvi)))
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
        }
    }
    return dwVersion;
}

STDMETHODIMP CPE32PropsView::EnumPE32VersionStringTableStrings(__in unsigned short wLanguage, __in unsigned short wCodePage, __in BSTR bstrKey, __in BSTR bstrValue, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);

	if (pContext)
	{
		CComBSTR Key;
		CComBSTR Value;

		PRES_TYPE_ENUM_VERSION_STRINGS pRTVS(reinterpret_cast<PRES_TYPE_ENUM_VERSION_STRINGS>(pContext));
		HTREEITEM hKeyTreeItem(nullptr);

		if (bstrKey)
		{
			Key.Attach(bstrKey);
			hKeyTreeItem = AddItemToTreeView(pRTVS ->hTree, pRTVS ->hTreeRoot, TVI_LAST, Key);
		}

		if (bstrValue)
		{
			Value.Attach(bstrValue);
			if (hKeyTreeItem)
				AddItemToTreeView(pRTVS ->hTree, hKeyTreeItem, TVI_LAST, Value);
			else
				AddItemToTreeView(pRTVS ->hTree, pRTVS ->hTreeRoot, TVI_LAST, Value);
		}

		Key.Empty();
		Value.Empty();

		hResult = S_OK;
	}

	return hResult;
	UNREFERENCED_PARAMETER(wLanguage);
	UNREFERENCED_PARAMETER(wCodePage);
}