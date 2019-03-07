#pragma warning( disable : 4995 )

#include "stdafx.h"
#include <WindowsX.h>
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <strsafe.h>
#include <memory>

INT_PTR CALLBACK CPE32PropsView::ChildManifestViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hEdit(::GetDlgItem(hDlg, IDC_EDT_MANIFEST));

			if (::IsThemeActive())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hEdit, VSCLASS_EDIT, nullptr);
			}
			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.right;
			::MoveWindow(hEdit, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);

			HRESULT hr;
			
			DWORD dwManifestSize(0);
			void* pManifestContent(nullptr);

			if (SUCCEEDED(hr = pView ->mf_pPE32 ->GetManifestContent(&dwManifestSize, &pManifestContent)) && pManifestContent && dwManifestSize)
			{
				//	Need to convert to unicode
				char* pszManifest(reinterpret_cast<char*>(pManifestContent));
				int cniBufferLen(0);

				if (0 != (cniBufferLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszManifest, static_cast<int>(dwManifestSize), nullptr, 0)))
				{
					auto Deleter1 = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
					std::unique_ptr<__wchar_t, decltype(Deleter1)> pszTmp(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) * (cniBufferLen + 1))), Deleter1);

					if (pszTmp)
					{
						::ZeroMemory(pszTmp.get(), (cniBufferLen + 1) * sizeof(__wchar_t));
						if (0 != ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszManifest, static_cast<int>(dwManifestSize), pszTmp.get(), cniBufferLen))
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
				}
			}
		}
		return 1;
	}
	return 0;
}

