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

typedef struct __tagEXPORT_ENUM_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hReport;
}
EXPORT_ENUM_CONTEXT, *PEXPORT_ENUM_CONTEXT;

void CPE32PropsView::MakeReportView(__in HWND hListView, __in LPCWSTR lpwTitles[], __in int iTitlesCount)
{
	RECT rc = { 0 };
	DWORD dwStyle(static_cast<DWORD>(::GetWindowLongW(hListView, GWL_STYLE)));
	::SetWindowLongW(hListView, GWL_STYLE, dwStyle | LVS_REPORT);
	::SetWindowPos(hListView, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	::GetClientRect(hListView, &rc);

	for (int iIndex(0); iIndex < iTitlesCount; ++iIndex)
	{
		LVCOLUMNW lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = const_cast<LPWSTR>(lpwTitles[iIndex]);
		lvc.cx = (rc.right - rc.left) / iTitlesCount;
		if (iIndex % 2)
			lvc.fmt = LVCFMT_LEFT;
		else
			lvc.fmt = LVCFMT_RIGHT;
		ListView_InsertColumn(hListView, iIndex, &lvc);
	}
}

void CPE32PropsView::AddStringToReportView(__in HWND hListView, __in LPCWSTR lpwszStr[], int nColumnsCount)
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
	::StringCchLengthW(lvi.pszText, STRSAFE_MAX_CCH, reinterpret_cast<size_t*>(&lvi.cchTextMax));
	int nStringNo = ListView_InsertItem(hListView, &lvi);

	//	Fill other columns
	for (int iIndex(1); iIndex < nColumnsCount; ++iIndex)
		ListView_SetItemText(hListView, nStringNo, iIndex, const_cast<LPWSTR>(lpwszStr[iIndex]));
}

INT_PTR CALLBACK CPE32PropsView::ChildExportDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hReport(::GetDlgItem(hDlg, IDC_EXPORTS));
			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hReport, VSCLASS_EXPLORER, nullptr);
			}

			CResString Symbol(IDS_EXPORT_SYMBOL);
			CResString Ordinal(IDS_EXPORT_ORDINAL);
			__wchar_t* lpwszExportReportTitles[2] = { Symbol.m_bszResString.m_str, Ordinal.m_bszResString.m_str };

			MakeReportView(hReport, (LPCWSTR*)(&lpwszExportReportTitles[0]), 2);

			EXPORT_ENUM_CONTEXT eec;
			eec.dwStructSize = sizeof(EXPORT_ENUM_CONTEXT);
			eec.pMainDlg = pView;
			eec.hDlg = hDlg;
			eec.hReport = hReport;

			int nTotal(0);
			pView ->mf_pPE32 ->EnumPEImgExports(dynamic_cast<IPE32ExportsCallback*>(pView), &eec, &nTotal);

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hReport, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
		}
		return 1;
	}
	return 0;
}

STDMETHODIMP CPE32PropsView::EnumPE32ExportSymbols(__in unsigned long nIndex, __in __PEIMGEXPORTITEM* pItemDesc, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);

	PEXPORT_ENUM_CONTEXT pEEC(reinterpret_cast<PEXPORT_ENUM_CONTEXT>(pContext));
	if (pEEC && sizeof(EXPORT_ENUM_CONTEXT) == pEEC ->dwStructSize)
	{
		hResult = S_OK;
		if (pEEC ->pMainDlg && ::IsWindow(pEEC ->hDlg) && ::IsWindow(pEEC ->hReport))
		{
			//	Add string item to report view ...
			if (fsForwarded == pItemDesc ->ForwardState)
			{
				//	Symbol is forwarded
				__wchar_t wszSymbol[MAX_PATH + 1] = { 0 };
				size_t nFwdNameLen(0);
				HRESULT hr(::StringCchLengthA((STRSAFE_PCNZCH)(pItemDesc ->pForwardedName), MAX_PATH, &nFwdNameLen));

				if (SUCCEEDED(hr) && nFwdNameLen)
				{
					UINT dwcnWChars(static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPSTR>(pItemDesc ->pForwardedName), -1, nullptr, 0)));
					if (dwcnWChars && (dwcnWChars < MAX_PATH) && ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<LPSTR>(pItemDesc ->pForwardedName), -1, &wszSymbol[0], MAX_PATH))
					{
						__wchar_t wszOrdinal[64] = { 0 };
						if (SUCCEEDED(::StringCchPrintfW(wszOrdinal, 64, L"#%u (Fwd)", pItemDesc ->dwOrdinal)))
						{
							__wchar_t* wszReportItem[] = { &wszSymbol[0], &wszOrdinal[0] };
							AddStringToReportView(pEEC ->hReport, (LPCWSTR*)(&wszReportItem[0]), ARRAYSIZE(wszReportItem));
						}
					}
				}
			}
			else
			{
				__wchar_t wszSymbol[MAX_PATH + 1] = { 0 };
				size_t nNameLen(0);
				if (pItemDesc ->pAnsiName)
				{
					HRESULT hr(::StringCchLengthA(reinterpret_cast<STRSAFE_PCNZCH>(pItemDesc ->pAnsiName), MAX_PATH, &nNameLen));
					if (SUCCEEDED(hr) && nNameLen)
					{
						UINT dwcnWChars(static_cast<UINT>(::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<LPSTR>(pItemDesc ->pAnsiName), -1, nullptr, 0)));
						if (dwcnWChars && (dwcnWChars < MAX_PATH) && ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<LPSTR>(pItemDesc ->pAnsiName), -1, &wszSymbol[0], MAX_PATH))
						{
							__wchar_t wszOrdinal[64] = { 0 };
							if (SUCCEEDED(::StringCchPrintfW(wszOrdinal, 64, L"#%u", pItemDesc ->dwOrdinal)))
							{
								__wchar_t* wszReportItem[] = { &wszSymbol[0], &wszOrdinal[0] };
								AddStringToReportView(pEEC ->hReport, (LPCWSTR*)(&wszReportItem[0]), ARRAYSIZE(wszReportItem));
							}
						}
					}
				}
				else
				{
					__wchar_t wszOrdinal[64] = { 0 };
					wszSymbol[0] = L' ';
					if (SUCCEEDED(::StringCchPrintfW(wszOrdinal, 64, L"#%u", pItemDesc ->dwOrdinal)))
					{
						__wchar_t* wszReportItem[] = { &wszSymbol[0], &wszOrdinal[0] };
						AddStringToReportView(pEEC ->hReport, (LPCWSTR*)(&wszReportItem[0]), ARRAYSIZE(wszReportItem));
					}
				}
			}
		}
	}

	return hResult;
}
