#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <uxtheme.h>
#include <vssym32.h>

extern HINSTANCE hDLLInstance;

typedef struct __tagCERT_ENUM_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hReport;
}
CERT_ENUM_CONTEXT, *PCERT_ENUM_CONTEXT;

const __wchar_t wszPropName[] = L"PE32PropsView_AtlDlg";
const __wchar_t wszWintrust[] = L"Wintrust.dll";

#define TMP_BUFFER_LEN								256
#define BUFFER_LEN									4096

INT_PTR CALLBACK CPE32PropsView::ChildSecurityDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));

			::SetPropW(hDlg, wszPropName, pView); 
			HWND hReport(::GetDlgItem(hDlg, IDC_CERT_REPORT));
			HWND hButton(::GetDlgItem(hDlg, IDVERIFYSIGN));

			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hReport, VSCLASS_EXPLORER, nullptr);
				::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left/* + ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.top/* + ::GetSystemMetrics(SM_CYDLGFRAME)*/, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left/* - 2 * ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top/* - 2 * ::GetSystemMetrics(SM_CYDLGFRAME)*/, FALSE);

			CResString VerifyBtnText(IDS_VERIFY_SIGN_TXT);
			::SetWindowTextW(hButton, VerifyBtnText.m_bszResString);

			PIMAGE_DATA_DIRECTORY pImgDataDir(nullptr);
			pView ->mf_pPE32 ->GetDataDirPtr(&pImgDataDir);

			if (pImgDataDir[IMAGE_DIRECTORY_ENTRY_SECURITY].Size && pImgDataDir[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress)
			{
				CResString PropName(IDS_CERT_INDEX);
				CResString PropVal(IDS_CERT_DESC);
				__wchar_t* lpwszSecReportItem[2] = { PropName.m_bszResString, PropVal.m_bszResString };

				MakeReportView(hReport, (LPCWSTR*)(&lpwszSecReportItem[0]), 2);

				CERT_ENUM_CONTEXT cec;
				cec.dwStructSize = sizeof(CERT_ENUM_CONTEXT);
				cec.pMainDlg = pView;
				cec.hDlg = hDlg;
				cec.hReport = hReport;

				int nTotal(0);
				pView ->mf_pPE32 ->EnumPEImgCertificates(dynamic_cast<IPE32CertificatesCallback*>(pView), &cec, &nTotal);
			}

			RECT rcOwner, rcButton;

			::GetWindowRect(hButton, &rcButton); 
			::GetClientRect(hDlg, &rcOwner);
			--rcOwner.right;
			--rcOwner.bottom;
			::OffsetRect(&rcOwner, -rcOwner.left, -rcOwner.top);
			::MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)(&rcButton), (sizeof(RECT) / sizeof(POINT)));
			::OffsetRect(&rcButton, -rcButton.left, -rcButton.top);
			::MoveWindow(hButton, rcOwner.right - rcButton.right, rcOwner.bottom - rcButton.bottom, rcButton.right - rcButton.left, rcButton.bottom - rcButton.top, FALSE);

			::GetWindowRect(hButton, &rcButton);
			::MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)(&rcButton), (sizeof(RECT) / sizeof(POINT)));
			::MoveWindow(hReport, rcOwner.left, rcOwner.top, rcOwner.right - rcOwner.left, rcButton.top - 2 * ::GetSystemMetrics(SM_CYDLGFRAME), FALSE);
		}
		return 1;
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case IDVERIFYSIGN:
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, wszPropName)));
				if (pView)
				{
					SYSTEMTIME SigDate = { 0 };
					SYSTEMTIME TimeStampDate = { 0 };
					CComBSTR FileSigner;
					CComBSTR TimestampSigner;
					CComBSTR Description;

					HRESULT hResult(pView ->mf_pPE32 ->VerifyPEImageSignatureUI(pView ->m_hWnd, &SigDate, &TimeStampDate, &FileSigner.m_str, &TimestampSigner.m_str));
					bool fbError(false);
					
					if (hResult != ERROR_SUCCESS)
					{
						DWORD dwErrorCode(::GetLastError());
						__wchar_t* lpBuffer(nullptr);
						//	Try to obtain error description from system
						if (HRESULT_FACILITY(hResult) == FACILITY_PE32VIEW)
						{
							if (::FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER, hDLLInstance, hResult, LANGIDFROMLCID(::GetThreadLocale()), reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr))
							{
								Description = lpBuffer;
								::LocalFree(lpBuffer);
							}
						}
						else
						{						
							if (::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, HRESULT_CODE(hResult), LANGIDFROMLCID(::GetThreadLocale()), reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr))
							{
								Description = lpBuffer;
								::LocalFree(lpBuffer);
							}
							else
							{
								//Wintrust.dll
								bool fbNeedToUnload(false);
								HMODULE hModule(nullptr);
								if (nullptr == (hModule = ::GetModuleHandleW(wszWintrust)))
								{
									if (hModule = ::LoadLibraryW(wszWintrust))
										fbNeedToUnload = true;
								}

								if (hModule && ::FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER, hModule, hResult, LANGIDFROMLCID(::GetThreadLocale()), reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr))
								{
									Description = lpBuffer;
									::LocalFree(lpBuffer);
								}
								else
								{
									//	Unable to obtain error description ...
									switch(hResult)
									{
										case TRUST_E_NOSIGNATURE:
											switch(dwErrorCode)
											{
											case TRUST_E_NOSIGNATURE:
											case TRUST_E_SUBJECT_FORM_UNKNOWN:
											case TRUST_E_PROVIDER_UNKNOWN:
												//	No signature
												{
													CResString NoSign(IDS_NO_EMBED_SIGNATURE);
													Description.Attach(NoSign.m_bszResString.Detach());
												}
												break;
											default:
												//	Unknown error
												FormatResource(Description, IDS_EMBED_SIGN_ERROR_UNKNOWN, hResult);
												break;
											}
										break;
										case TRUST_E_EXPLICIT_DISTRUST:
											{
												CResString NotAllowed(IDS_EMBED_SIGN_DISALLOWED);
												Description.Attach(NotAllowed.m_bszResString.Detach());
											}
											break;
										case TRUST_E_SUBJECT_NOT_TRUSTED:
											{
												CResString NotTrusted(IDS_EMBED_SIGN_NOT_TRUSTED);
												Description.Attach(NotTrusted.m_bszResString.Detach());
											}
											break;
										case CRYPT_E_SECURITY_SETTINGS:
											{
												CResString SecuritySettings(IDS_EMBED_SIGN_SECURITY_SETTINGS);
												Description.Attach(SecuritySettings.m_bszResString.Detach());
												fbError = true;
											}
											break;
										default:
											{
												CResString DisabledByAdminPolicy(IDS_EMBED_SIGN_DISABLED_BY_ADMIN_POLICY);
												Description.Attach(DisabledByAdminPolicy.m_bszResString.Detach());
												fbError = true;
											}
											break;
									}
								}
								if (fbNeedToUnload)
									::FreeLibrary(hModule);
							}
						}
					}
					else
					{
						CResString SignOk(IDS_EMBED_SIGN_VERIFY_OK);
						Description.Attach(SignOk.m_bszResString.Detach());
					}

					__wchar_t wszContent[BUFFER_LEN] = { 0 };
					__wchar_t lpwszSignBuffTmp[TMP_BUFFER_LEN] = { 0 };
					__wchar_t lpwszTimestampBuffTmp[TMP_BUFFER_LEN] = { 0 };

					CResString SignDate(IDS_EMBED_SIGN_DATE);
					CResString TimestampDate(IDS_EMBED_SIGN_TIMESTAMP);

					::StringCchPrintfW(lpwszSignBuffTmp, TMP_BUFFER_LEN, SignDate.m_bszResString.m_str, SigDate.wDay, SigDate.wMonth, SigDate.wYear, SigDate.wHour, SigDate.wMinute, SigDate.wSecond);
					::StringCchPrintfW(lpwszTimestampBuffTmp, TMP_BUFFER_LEN, TimestampDate.m_bszResString.m_str, TimeStampDate.wDay, TimeStampDate.wMonth, TimeStampDate.wYear, TimeStampDate.wHour, TimeStampDate.wMinute, TimeStampDate.wSecond);

					::StringCchPrintfW(&wszContent[0], 4096, L"\n%s\nSigner: %s\n\n%sSigner: %s\n", lpwszSignBuffTmp, FileSigner, lpwszTimestampBuffTmp, TimestampSigner);
					if (!fbError)
						::TaskDialog(pView ->m_hWnd, hDLLInstance, MAKEINTRESOURCEW(IDS_SIGN_VERIFY_RESULTS), Description.m_str, wszContent, TDCBF_OK_BUTTON, MAKEINTRESOURCEW(IDI_DLGLOGOICONEX), nullptr);
					else
						::TaskDialog(pView ->m_hWnd, hDLLInstance, MAKEINTRESOURCEW(IDS_SIGN_VERIFY_RESULTS), Description.m_str, nullptr, TDCBF_OK_BUTTON, MAKEINTRESOURCEW(IDI_DLGLOGOICONEX), nullptr);
				}
				break;
			}
		}
		break;
	}
	return 0;
}

STDMETHODIMP CPE32PropsView::EnumPE32Certificates(__in unsigned long dwIndex, __in BSTR pwszCertSubject, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);

	CComBSTR CertSubject;
	CertSubject.Attach(pwszCertSubject);

	PCERT_ENUM_CONTEXT pCEC(reinterpret_cast<PCERT_ENUM_CONTEXT>(pContext));
	if (pCEC && sizeof(CERT_ENUM_CONTEXT) == pCEC ->dwStructSize)
	{
		hResult = S_OK;
		if (pCEC ->pMainDlg && ::IsWindow(pCEC ->hDlg) && ::IsWindow(pCEC ->hReport))
		{
			__wchar_t wszIndex[64] = { 0 };
			if (SUCCEEDED(::StringCchPrintfW(wszIndex, 64, L"#%u", dwIndex)))
			{
				__wchar_t* wszReportItem[2] = { &wszIndex[0], CertSubject.m_str };
				AddStringToReportView(pCEC ->hReport, (LPCWSTR*)(&wszReportItem[0]), 2);
			}
		}
	}
	return hResult;
}

