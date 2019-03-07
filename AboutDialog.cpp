#pragma warning( disable : 4995 )

#include "stdafx.h"
//#include <WindowsX.h>
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <memory>
#pragma comment(lib, "Msimg32.lib")

extern HINSTANCE hDLLInstance;

typedef struct __tagABOUT_INFO
{
	DWORD cbStructSize;
	HICON hIcon;
	SIZE sizeIcon;
	DWORD dwBitsPerPixel;
	BSTR bszName;
	size_t cchNameLen;
	BSTR bszVer;
	size_t cchVerLen;
	BSTR bszAutor;
	size_t cchAuthorLen;
	HTHEME hTheme;
	HTHEME hThemeTxt;
}
ABOUT_INFO, *PABOUT_INFO;

void ClearAboutInfoStruct(__in PABOUT_INFO pAI)
{
	if (pAI && sizeof(ABOUT_INFO) == pAI ->cbStructSize)
	{
		if (pAI ->hIcon && ::DestroyIcon(pAI ->hIcon))
			pAI ->hIcon = nullptr;
		if (pAI ->bszName)
		{
			::SysFreeStringHelper(pAI ->bszName);
			pAI ->bszName = nullptr;
		}
		if (pAI ->bszVer)
		{
			::SysFreeStringHelper(pAI ->bszVer);
			pAI ->bszVer = nullptr;
		}
		if (pAI ->bszAutor)
		{
			::SysFreeStringHelper(pAI ->bszAutor);
			pAI ->bszAutor = nullptr;
		}
		if (pAI ->hTheme)
		{
			::CloseThemeData(pAI ->hTheme);
			pAI ->hTheme = nullptr;
		}
		if (pAI ->hThemeTxt)
		{
			::CloseThemeData(pAI ->hThemeTxt);
			pAI ->hThemeTxt = nullptr;
		}
	}
}

const __wchar_t szAboutInfo[] = L"PE32ViewerAboutDlgProp_Resources";

INT_PTR CALLBACK CPE32PropsView::ChildAboutDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			::BufferedPaintInit();
			if (::IsThemeActive() && ::IsAppThemed())
				::SetWindowTheme(hDlg, VSCLASS_TASKDIALOG, nullptr);

			auto Deleter = [](PABOUT_INFO pai) { if (pai) ::CoTaskMemFree(pai); };
			std::unique_ptr<ABOUT_INFO, decltype(Deleter)> pAI(reinterpret_cast<ABOUT_INFO*>(::CoTaskMemAlloc(sizeof(ABOUT_INFO))), Deleter);

			if (pAI)
			{
				::ZeroMemory(pAI.get(), sizeof(ABOUT_INFO));
				pAI ->cbStructSize = sizeof(ABOUT_INFO);

				if (::IsThemeActive() && ::IsAppThemed())
				{
					pAI ->hTheme = ::OpenThemeData(hDlg, VSCLASS_TASKDIALOG);
					pAI ->hThemeTxt = ::OpenThemeData(hDlg, VSCLASS_TEXTSTYLE);
				}

				ICONINFO ii = { 0 };

				HICON hIcon(reinterpret_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_DLGLOGOICONEX), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR)));
				if (hIcon && ::GetIconInfo(hIcon, &ii))
				{
					pAI ->hIcon = hIcon;

					BITMAP bm = { 0 };
					if (ii.hbmColor && sizeof(BITMAP) == ::GetObjectW(ii.hbmColor, sizeof(BITMAP), &bm))
					{
						pAI ->sizeIcon.cx = bm.bmWidth;
						pAI ->sizeIcon.cy = bm.bmHeight < 0 ? -bm.bmHeight : bm.bmHeight;
						pAI ->dwBitsPerPixel = bm.bmBitsPixel;
					}

					if (ii.hbmColor)
						::DeleteObject(ii.hbmColor);
					if (ii.hbmMask)
						::DeleteObject(ii.hbmMask);
				}

				CResString Name(IDS_RES_PROJ_DISPLAY_NAME);
				pAI ->bszName = Name.m_bszResString.Detach();
				if (pAI ->bszName)
					::StringCchLengthW(pAI ->bszName, STRSAFE_MAX_CCH, &pAI ->cchNameLen);

				CResString Version(IDS_RES_PROJ_VER_DISPLAY_NAME);
				pAI ->bszVer = Version.m_bszResString.Detach();
				if (pAI ->bszVer)
					::StringCchLengthW(pAI ->bszVer, STRSAFE_MAX_CCH, &pAI ->cchVerLen);

				CResString Author(IDS_RES_PROJ_AUTHOR);
				pAI ->bszAutor = Author.m_bszResString.Detach();
				if (pAI ->bszAutor)
					::StringCchLengthW(pAI ->bszAutor, STRSAFE_MAX_CCH, &pAI ->cchAuthorLen);

				if (!::SetPropW(hDlg, szAboutInfo, pAI.get()))
					ClearAboutInfoStruct(pAI.get());
				else
					pAI.release();
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			::SetWindowPos(hDlg, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		return 1;
	case WM_DESTROY:
		{
			PABOUT_INFO pAIStruct(reinterpret_cast<PABOUT_INFO>(::RemovePropW(hDlg, szAboutInfo)));
			if (pAIStruct)
			{
				auto Deleter = [](PABOUT_INFO pai) { if (pai) ::CoTaskMemFree(pai); };
				std::unique_ptr<ABOUT_INFO, decltype(Deleter)> pAI(pAIStruct, Deleter);
				ClearAboutInfoStruct(pAI.get());
			}
			::BufferedPaintUnInit();
		}
		break;
	case WM_PAINT:
		{
			PABOUT_INFO pAIStruct(reinterpret_cast<PABOUT_INFO>(::GetPropW(hDlg, szAboutInfo)));
			if (pAIStruct)
			{
				RECT Rect;
				::GetClientRect(hDlg, &Rect);
				--Rect.right;
				--Rect.bottom;

				PAINTSTRUCT ps;
				HDC hDC(::BeginPaint(hDlg, &ps));
				if (hDC)
				{
					if (pAIStruct ->hTheme && pAIStruct ->hThemeTxt)
					{
						BP_PAINTPARAMS paintParams = {0};
						paintParams.cbSize = sizeof(paintParams);

						HDC hDCMem;  
						HPAINTBUFFER hPB(::BeginBufferedPaint(hDC, &Rect, BPBF_TOPDOWNDIB, &paintParams, &hDCMem));
						if (hPB)
						{
							::BufferedPaintClear(hPB, nullptr);

							if ((::IsThemeActive() && ::IsAppThemed()) && ::IsThemeBackgroundPartiallyTransparent(pAIStruct ->hTheme, TDLG_MAININSTRUCTIONPANE, 0))
								::DrawThemeParentBackground(hDlg, hDCMem, &Rect);

							if ((::IsThemeActive() && ::IsAppThemed()))
							{
								if (FAILED(::DrawThemeBackground(pAIStruct ->hTheme, hDCMem, TDLG_MAININSTRUCTIONPANE, 0, &Rect, &ps.rcPaint)))
								{
									COLORREF cr(::GetSysColor(COLOR_WINDOW));
									if (FAILED(::GetThemeColor(pAIStruct ->hTheme, TDLG_MAININSTRUCTIONPANE, 0, TMT_COLOR, &cr)))
										::FillRect(hDCMem,  &Rect, ::GetSysColorBrush(COLOR_WINDOW));
									else
									{
										HBRUSH hBrush(::CreateSolidBrush(cr));
										if (hBrush)
										{
											::FillRect(hDCMem,  &Rect, hBrush);
											::DeleteObject(hBrush);
										}
									}
								}
							}
							else
							{
								COLORREF cr(::GetSysColor(COLOR_WINDOW));
								HBRUSH hBrush(::CreateSolidBrush(cr));
								if (hBrush)
								{
									::FillRect(hDCMem,  &Rect, hBrush);
									::DeleteObject(hBrush);
								}
							}


							LONG lnDlgBaseUnit(::GetDialogBaseUnits());
							ULONG lnXUnit(LOWORD(lnDlgBaseUnit));
							ULONG lnYUnit(HIWORD(lnDlgBaseUnit));

							ULONG lcX(lnXUnit), lcY(lnYUnit);

							if (pAIStruct ->hIcon)
							{
								RECT rcTmp;
								::CopyRect(&rcTmp, &Rect);
								::OffsetRect(&rcTmp, lnXUnit, lnYUnit);

								ICONINFO iconinfo = { 0 };
								if (::GetIconInfo(pAIStruct ->hIcon, &iconinfo))
								{
									BITMAP bmp = { 0 };
									HDC hMemDC(::CreateCompatibleDC(hDCMem));
									if (hMemDC && sizeof(BITMAP) == ::GetObjectW(iconinfo.hbmMask, sizeof(BITMAP), &bmp))
									{
										HGDIOBJ hOld(::SelectObject(hMemDC, iconinfo.hbmColor));
										COLORREF crTrans(::GetPixel(hMemDC, 0, 0));
										if (!::TransparentBlt(hDCMem, lnXUnit, lnYUnit, bmp.bmWidth, bmp.bmHeight, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, crTrans))
											::MaskBlt(hDCMem, lnXUnit, lnYUnit, bmp.bmWidth, bmp.bmHeight, hMemDC, 0, 0, iconinfo.hbmMask, 0, 0, MAKEROP4(SRCINVERT, SRCCOPY));
										lcX += static_cast<ULONG>(pAIStruct ->sizeIcon.cx);
										::SelectObject(hMemDC, hOld);
									}

									if (iconinfo.hbmMask)
										::DeleteObject(iconinfo.hbmMask);
									if (iconinfo.hbmColor)
										::DeleteObject(iconinfo.hbmColor);
									if (hMemDC)
										::DeleteObject(hMemDC);
								}

								lcX += lnXUnit;
							}

							DTTOPTS DTTOpts = { 0 };
							DTTOpts.dwSize = sizeof(DTTOPTS);
							DTTOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR | DTT_APPLYOVERLAY | DTT_SHADOWTYPE | DTT_SHADOWCOLOR;
							DTTOpts.iGlowSize = 8;
							DTTOpts.iTextShadowType = TST_CONTINUOUS;
							DTTOpts.crShadow = ::GetThemeSysColor(pAIStruct ->hTheme, COLOR_BTNSHADOW) & 0xFFFFFF;

							if (FAILED(::GetThemeColor(pAIStruct ->hThemeTxt, TEXT_MAININSTRUCTION, 0, TMT_TEXTCOLOR, &DTTOpts.crText)))
								DTTOpts.crText = ::GetSysColor(COLOR_WINDOWTEXT) & 0xFFFFFF;

							if (pAIStruct ->bszName)
							{
								RECT rcTxt = { 0 };
								if (S_OK == ::GetThemeTextExtent(pAIStruct ->hThemeTxt, hDCMem, TEXT_MAININSTRUCTION, 0, pAIStruct ->bszName, -1, DT_SINGLELINE | DT_LEFT| DT_NOPREFIX, &Rect, &rcTxt))
								{
									rcTxt.left = lcX;
									rcTxt.top = lcY;
									rcTxt.right = Rect.right;
									rcTxt.bottom += lcY;
									if (S_OK  == ::DrawThemeTextEx(pAIStruct ->hThemeTxt, hDCMem, TEXT_MAININSTRUCTION, 0, pAIStruct ->bszName, -1, DT_SINGLELINE | DT_LEFT| DT_NOPREFIX, &rcTxt, &DTTOpts))
										lcY += rcTxt.bottom - rcTxt.top;
								}
							}

							if (pAIStruct ->bszVer)
							{
								RECT rcTxt = { 0 };
								if (S_OK == ::GetThemeTextExtent(pAIStruct ->hThemeTxt, hDCMem, TEXT_INSTRUCTION, 0, pAIStruct ->bszVer, -1, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX, &Rect, &rcTxt))
								{
									rcTxt.left = lcX;
									rcTxt.top = lcY;
									rcTxt.right = Rect.right;
									rcTxt.bottom += lcY;
									if (S_OK  == ::DrawThemeTextEx(pAIStruct ->hThemeTxt, hDCMem, TEXT_INSTRUCTION, 0, pAIStruct ->bszVer, -1, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX, &rcTxt, &DTTOpts))
										lcY += rcTxt.bottom - rcTxt.top;
								}
							}

							if (pAIStruct ->bszAutor)
							{
								RECT rcTxt = { 0 };
								if (S_OK == ::GetThemeTextExtent(pAIStruct ->hThemeTxt, hDCMem, TEXT_BODYTEXT, 0, pAIStruct ->bszAutor, -1, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX, &Rect, &rcTxt))
								{
									rcTxt.left = lcX;
									rcTxt.top = lcY;
									rcTxt.right = Rect.right;
									rcTxt.bottom += lcY;
									if (S_OK  == ::DrawThemeTextEx(pAIStruct ->hThemeTxt, hDCMem, TEXT_BODYTEXT, 0, pAIStruct ->bszAutor, -1, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX, &rcTxt, &DTTOpts))
										lcY += rcTxt.bottom - rcTxt.top;
								}
							}
							::BitBlt (hDC, 0, 0, Rect.right, Rect.bottom, hDCMem, 0, 0, SRCCOPY);
							::EndBufferedPaint(hPB, TRUE);
						}
					}
					else
					{
						HDC hDCMem(::CreateCompatibleDC(hDC));
						if (hDCMem)
						{
							RECT Rect;
							::GetClientRect(hDlg, &Rect);
							--Rect.right;
							--Rect.bottom;

							LONG lnWidth(Rect.right - Rect.left);
							LONG lnHeight(Rect.bottom - Rect.top);

							HBITMAP hBmp(::CreateCompatibleBitmap(hDC, lnWidth, lnHeight));
							if (hBmp)
							{
								NONCLIENTMETRICSW ncm = {0};
								ncm.cbSize = sizeof(NONCLIENTMETRICSW);

								HFONT hFont(nullptr);
								HGDIOBJ hFontOriginal(nullptr);

								if (!::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0))
									hFontOriginal = ::SelectObject(hDCMem, ::GetStockObject(DEFAULT_GUI_FONT));
								else
								{
									hFont = ::CreateFontIndirectW(&ncm.lfCaptionFont);
									if (hFont)
										hFontOriginal = ::SelectObject(hDCMem, hFont);
									else
										hFontOriginal = ::SelectObject(hDCMem, ::GetStockObject(DEFAULT_GUI_FONT));
								}

								HGDIOBJ hBmpOriginal(::SelectObject(hDCMem, hBmp));
								HBRUSH hBrush(::CreateSolidBrush(::GetSysColor(COLOR_WINDOW)));

								if (hBrush)
								{
									::FillRect(hDCMem,  &Rect, hBrush);
									::DeleteObject(hBrush);
								}

								::SetBkColor(hDCMem, ::GetSysColor(COLOR_WINDOW));
								::SetTextColor(hDCMem, ::GetSysColor(COLOR_MENUTEXT)/* & 0xFFFFFF*/);

								LONG lnDlgBaseUnit(::GetDialogBaseUnits());
								ULONG lnXUnit(LOWORD(lnDlgBaseUnit));
								ULONG lnYUnit(HIWORD(lnDlgBaseUnit));

								ULONG lcX(lnXUnit), lcY(lnYUnit);

								if (pAIStruct ->hIcon)
								{
									RECT rcTmp;
									::CopyRect(&rcTmp, &Rect);
									::OffsetRect(&rcTmp, lnXUnit, lnYUnit);

									ICONINFO iconinfo = { 0 };
									if (::GetIconInfo(pAIStruct ->hIcon, &iconinfo))
									{
										BITMAP bmp = { 0 };
										HDC hMemDC(::CreateCompatibleDC(hDC));//hDCMem
										if (hMemDC && sizeof(BITMAP) == ::GetObjectW(iconinfo.hbmMask, sizeof(BITMAP), &bmp))
										{
											HGDIOBJ hOld(::SelectObject(hMemDC, iconinfo.hbmColor));
											COLORREF crTrans(::GetPixel(hMemDC, 0, 0));
											if (!::TransparentBlt(hDCMem, lnXUnit, lnYUnit, bmp.bmWidth, bmp.bmHeight, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, crTrans))
												::MaskBlt(hDCMem, lnXUnit, lnYUnit, bmp.bmWidth, bmp.bmHeight, hMemDC, 0, 0, iconinfo.hbmMask, 0, 0, MAKEROP4(SRCINVERT, SRCCOPY));
											lcX += static_cast<ULONG>(pAIStruct ->sizeIcon.cx);
											::SelectObject(hMemDC, hOld);
										}

										if (iconinfo.hbmMask)
											::DeleteObject(iconinfo.hbmMask);
										if (iconinfo.hbmColor)
											::DeleteObject(iconinfo.hbmColor);
										if (hMemDC)
											::DeleteObject(hMemDC);
									}

									lcX += lnXUnit;
								}

								if (pAIStruct ->bszName && pAIStruct ->cchNameLen)
								{
									SIZE szTxt = { 0 };
									if (::GetTextExtentPoint32W(hDCMem, pAIStruct ->bszName, static_cast<int>(pAIStruct ->cchNameLen), &szTxt))
									{
										RECT rcTxt = { 0 };

										rcTxt.left = lcX;
										rcTxt.top = lcY;
										rcTxt.right = lnWidth;
										rcTxt.bottom = lcY + szTxt.cy;

										int nTextHeight(::DrawTextExW(hDCMem, pAIStruct ->bszName, -1, &rcTxt, DT_SINGLELINE | DT_LEFT| DT_NOPREFIX, nullptr));
										lcY += static_cast<ULONG>(nTextHeight);
									}
								}

								if (pAIStruct ->bszVer)
								{
									SIZE szTxt = { 0 };
									if (::GetTextExtentPoint32W(hDCMem, pAIStruct ->bszVer, static_cast<int>(pAIStruct ->cchVerLen), &szTxt))
									{
										RECT rcTxt = { 0 };
										rcTxt.left = lcX;
										rcTxt.top = lcY;
										rcTxt.right = lnWidth;
										rcTxt.bottom = lcY + szTxt.cy;

										int nTextHeight(::DrawTextExW(hDCMem, pAIStruct ->bszVer, -1, &rcTxt, DT_SINGLELINE | DT_LEFT| DT_NOPREFIX, nullptr));
										lcY += static_cast<ULONG>(nTextHeight);
									}
								}

								if (pAIStruct ->bszAutor)
								{
									SIZE szTxt = { 0 };
									if (::GetTextExtentPoint32W(hDCMem, pAIStruct ->bszAutor, static_cast<int>(pAIStruct ->cchAuthorLen), &szTxt))
									{
										RECT rcTxt = { 0 };
										rcTxt.left = lcX;
										rcTxt.top = lcY;
										rcTxt.right = lnWidth;
										rcTxt.bottom += lcY + szTxt.cy;

										int nTextHeight(::DrawTextExW(hDCMem, pAIStruct ->bszAutor, -1, &rcTxt, DT_SINGLELINE | DT_LEFT| DT_NOPREFIX, nullptr));
										lcY += static_cast<ULONG>(nTextHeight);
									}
								}

								::BitBlt (hDC, 0, 0, lnWidth, lnHeight, hDCMem, 0, 0, SRCCOPY);

								::SelectObject(hDCMem, hFontOriginal);
								::SelectObject(hDCMem, hBmpOriginal);
								if (hFont)
									::DeleteObject(hFont);
								::DeleteObject(hBmp);
							}
							::DeleteDC(hDCMem);
						}
					}
					::EndPaint(hDlg, &ps);
				}
			}
		}
		break;
	}
	return 0;
}
