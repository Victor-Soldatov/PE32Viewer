// XPE32Viewer.cpp : Implementation of CCoPE32Viewer
#pragma warning( disable : 4995 )

#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Misc.h"
#include "PE32Decoder.h"

#include <wincodec.h>
#include <memory>

extern volatile unsigned long g_nCount;

//	CCoPE32Viewer
CCoPE32Viewer::CCoPE32Viewer(void) : 
	mf_hInstance(hDLLInstance), 
	mf_hBmp(nullptr)
{
	::ZeroMemory(mf_szFileName, sizeof(__wchar_t) * (MAX_PATH + 1));
	::ZeroMemory(mf_szVerbW, sizeof(__wchar_t) * MAX_PATH);
	::StringCchCopyW(mf_szVerbW, MAX_PATH, L"PE32View");
	::ZeroMemory(mf_szVerbA, sizeof(char) * MAX_PATH);
	::StringCchCopyA(mf_szVerbA, MAX_PATH, "PE32View");

	mf_ptPoint.x = -1;
	mf_ptPoint.y = -1;
}

BSTR CCoPE32Viewer::LoadStringEx(__in HINSTANCE hInstance, __in DWORD MessageID, __in LANGID LangID)
{
	__wchar_t* lpStr(nullptr);
	HRSRC hResource(::FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(MessageID / 16 + 1), LangID));
	if (hResource) 
	{
		HGLOBAL hGlobal(::LoadResource(hInstance, hResource));
		if (hGlobal) 
		{
			const __wchar_t* pwszRes(reinterpret_cast<const __wchar_t*>(::LockResource(hGlobal)));
			if (pwszRes) 
			{
				for (DWORD i = 0; i < (MessageID & 15); ++i) 
					pwszRes += 1 + *(WORD*)(pwszRes);

				UnlockResource(pwszRes);

				BSTR pwszStr(::SysAllocStringLen(nullptr, *reinterpret_cast<WORD*>(const_cast<__wchar_t*>(pwszRes))));

				if (pwszStr != nullptr) 
				{
					pwszStr[*(WORD*)(pwszRes)] = L'\0';
					::CopyMemory(pwszStr, pwszRes+1, *(reinterpret_cast<WORD*>(const_cast<__wchar_t*>(pwszRes))) * sizeof(__wchar_t));
				}
				lpStr = pwszStr;
			}
			::FreeResource(hGlobal);
		}
	} 
	return lpStr;
}

char* CCoPE32Viewer::LoadAnsiStringEx(__in HINSTANCE hInstance, __in DWORD MessageID, __in LANGID LangID, __in UINT nCodePage)
{
	char* lptStr(nullptr);
	CComBSTR pwszText;
	pwszText.m_str = LoadStringEx(hInstance, MessageID, LangID);
	if (pwszText.m_str)
	{
		CPINFOEXA cpiEx = {0};
		if (::GetCPInfoExA(CP_ACP, 0, &cpiEx))
		{
			UINT uiCodePage(::GetACP());
			LPCSTR lpDefaultChar(((uiCodePage == CP_UTF7) || (uiCodePage == CP_UTF8)) ? nullptr : ((LPCSTR)(&cpiEx.DefaultChar)));
			BOOL fbDefaultCharIsUsed(FALSE);
			LPBOOL lpDefaultCharIsUsedFlag(((uiCodePage == CP_UTF7) || (uiCodePage == CP_UTF8)) ? nullptr : &fbDefaultCharIsUsed);

			int niTChars(::WideCharToMultiByte(uiCodePage, 0, pwszText.m_str, -1, nullptr, 0, lpDefaultChar, lpDefaultCharIsUsedFlag));
			if (niTChars != 0)
			{
				char* lpszAnsiStringBuffer(reinterpret_cast<char*>(::CoTaskMemAlloc(niTChars * sizeof(char))));
				if (lpszAnsiStringBuffer)
				{
					if (0 != ::WideCharToMultiByte(uiCodePage, 0, pwszText.m_str, -1, lpszAnsiStringBuffer, niTChars, lpDefaultChar, lpDefaultCharIsUsedFlag))
						lptStr = lpszAnsiStringBuffer;
					else
					{
						::CoTaskMemFree(lpszAnsiStringBuffer);
						lpszAnsiStringBuffer = nullptr;
					}
				}
			}
		}
		else
		{
			int niTChars(::WideCharToMultiByte(nCodePage, 0, pwszText.m_str, -1, nullptr, 0, nullptr, nullptr));
			if (niTChars)
			{
				char* lpszAnsiStringBuffer(reinterpret_cast<char*>(::CoTaskMemAlloc(niTChars * sizeof(char))));
				if (lpszAnsiStringBuffer)
				{
					if (0 != ::WideCharToMultiByte(nCodePage, 0, pwszText.m_str, -1, lpszAnsiStringBuffer, niTChars, nullptr, nullptr))
						lptStr = lpszAnsiStringBuffer;
					else
					{
						::CoTaskMemFree(lpszAnsiStringBuffer);
						lpszAnsiStringBuffer = nullptr;
					}
				}
			}
		}
	}
	return lptStr;
}

LCID CCoPE32Viewer::GetLocale(void)
{
	LCID lcid(::GetThreadLocale());
	if (LOCALE_CUSTOM_UNSPECIFIED == lcid)
		lcid = ::GetUserDefaultLCID();
	return lcid;
}

//	IPE32Viewer
STDMETHODIMP CCoPE32Viewer::GetVersion(__out LONG* plVersion)
{
	HRESULT hR(S_OK);
	if (plVersion)
		*plVersion = MAKELONG(PE32VIEW_VERSION_MINOR, PE32VIEW_VERSION_MAJOR);
	else
		hR = E_INVALIDARG;
	return hR;
}

STDMETHODIMP CCoPE32Viewer::GetVersionEx(__out PE32VIEW_VERSION* psVersionEx)
{
	HRESULT hR(S_OK);
	if (psVersionEx)
	{
		psVersionEx ->dwVerMajor = PE32VIEW_VERSION_MAJOR;
		psVersionEx ->dwVerMinor = PE32VIEW_VERSION_MINOR;
		psVersionEx ->dwVerBuildNo = PE32VIEW_VERSION_BUILD;
		psVersionEx ->dwVerSpecialNo = PE32VIEW_VERSION_SPECIAL_BUILD;
	}
	else
		hR = E_INVALIDARG;
	return hR;
}

//	IShellExtInit
STDMETHODIMP CCoPE32Viewer::Initialize(__in_opt PCIDLIST_ABSOLUTE pidlFolder, __in_opt IDataObject *pdtobj, __in_opt HKEY hkeyProgID)
{
	if (!pdtobj)
		return E_INVALIDARG;

	STGMEDIUM medium;
	FORMATETC fe;

	fe.cfFormat = CF_HDROP;
	fe.ptd = nullptr;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;	//1
	fe.tymed = TYMED_HGLOBAL;

	HRESULT hr;

	if (SUCCEEDED(hr = pdtobj ->GetData(&fe, &medium)))
	{
		HDROP hDrop(static_cast<HDROP>(::GlobalLock(medium.hGlobal)));
		if (!hDrop)
			hr = E_INVALIDARG;
		else
		{
			if (1 == ::DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0))
			{
				::ZeroMemory(mf_szFileName, sizeof(__wchar_t) * (MAX_PATH + 1));
				if (0 == ::DragQueryFileW(hDrop, 0, mf_szFileName, MAX_PATH))
					hr = E_INVALIDARG;
				else
				{
					INITCOMMONCONTROLSEX icc = { 0 };

					// Initialise common controls.
					icc.dwSize = sizeof(icc);
					icc.dwICC = ICC_WIN95_CLASSES;
					::InitCommonControlsEx(&icc);
				}
			}
			else
				hr = E_INVALIDARG;
		}

		if (!hDrop)
			::GlobalUnlock(medium.hGlobal);
		::ReleaseStgMedium(&medium);
	}
	else
		hr = E_INVALIDARG;

	return (hr);
}

//	IContextMenu
STDMETHODIMP CCoPE32Viewer::QueryContextMenu(__in HMENU hmenu, __in UINT indexMenu, __in UINT idCmdFirst, __in UINT idCmdLast, __in UINT uFlags)
{
	UINT idCmd(idCmdFirst);
	if (CMF_DEFAULTONLY != (CMF_DEFAULTONLY & uFlags))
	{
		CComBSTR pwszText;
		pwszText.m_str = LoadStringEx(mf_hInstance, IDS_MENUITEMTEXT, ::GetUserDefaultLangID());
		if (!pwszText.m_str)
			pwszText.m_str = LoadStringEx(mf_hInstance, IDS_MENUITEMTEXT, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));

		mf_nCmdID = idCmdFirst;
		mf_hMenu = hmenu;

		::InsertMenuW(hmenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, nullptr);
		mf_nMenuItemIndex = indexMenu;

		MENUITEMINFOW menuiteminfo = {0};
		menuiteminfo.cbSize = sizeof(menuiteminfo);
		menuiteminfo.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
		menuiteminfo.fType = MFT_STRING;
		menuiteminfo.dwTypeData = pwszText.m_str;

		if (!mf_hBmp)
		{
			HICON hIcon;
			if (FAILED(::LoadIconMetric(mf_hInstance, MAKEINTRESOURCE(IDI_DLGLOGOICON), LIM_SMALL, &hIcon)))
				hIcon = static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCE(IDI_DLGLOGOICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));

			if (hIcon)
			{
				CComPtr<IWICImagingFactory> pFactory;
				pFactory.CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
				if (pFactory.p)
					AddIconToMenuItem(pFactory.p, hIcon, FALSE, &mf_hBmp);
				if (!mf_hBmp)
					AddIconToMenuItemNoWIC(hIcon, FALSE, &mf_hBmp);
				if (!mf_hBmp)
					AddIconToMenuItemGDI(hIcon, FALSE, &mf_hBmp);
			}

			::DestroyIcon(hIcon);
		}

		if (mf_hBmp)
		{
			menuiteminfo.fMask |= MIIM_BITMAP;
			menuiteminfo.hbmpItem = mf_hBmp;
		}
		menuiteminfo.wID = mf_nCmdID;
		::InsertMenuItemW(hmenu, indexMenu++, TRUE, &menuiteminfo);
		::InsertMenuW(hmenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, nullptr);
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
	}
	else
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
}

HRESULT CCoPE32Viewer::ConvertBufferToPARGB32(HPAINTBUFFER hPaintBuffer, HDC hdc, HICON hicon, SIZE& sizIcon)
{
    RGBQUAD *prgbQuad;
    int cxRow;
    HRESULT hr = GetBufferedPaintBits(hPaintBuffer, &prgbQuad, &cxRow);
    if (SUCCEEDED(hr))
    {
        ARGB *pargb = reinterpret_cast<ARGB *>(prgbQuad);
        if (!HasAlpha(pargb, sizIcon, cxRow))
        {
            ICONINFO info;
            if (::GetIconInfo(hicon, &info))
            {
                if (info.hbmMask)
                    hr = ConvertToPARGB32(hdc, pargb, info.hbmMask, sizIcon, cxRow);

                ::DeleteObject(info.hbmColor);
                ::DeleteObject(info.hbmMask);
            }
        }
    }

    return hr;
}

void CCoPE32Viewer::InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp)
{
    ::ZeroMemory(pbmi, cbInfo);
    pbmi ->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi ->bmiHeader.biPlanes = 1;
    pbmi ->bmiHeader.biCompression = BI_RGB;

    pbmi ->bmiHeader.biWidth = cx;
    pbmi ->bmiHeader.biHeight = cy;
    pbmi ->bmiHeader.biBitCount = bpp;
}

HRESULT CCoPE32Viewer::Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp)
{
    *phBmp = nullptr;

    BITMAPINFO bmi;
    InitBitmapInfo(&bmi, sizeof(bmi), psize ->cx, psize ->cy, 32);

    HDC hdcUsed(hdc ? hdc : ::GetDC(nullptr));
    if (hdcUsed)
    {
        *phBmp = ::CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, nullptr, 0);
        if (hdc != hdcUsed)
            ::ReleaseDC(nullptr, hdcUsed);
    }
    return (nullptr == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

HRESULT CCoPE32Viewer::ConvertToPARGB32(__in HDC hdc, __inout ARGB *pargb, __in HBITMAP hbmp, __in SIZE& sizImage, __in int cxRow)
{
    BITMAPINFO bmi;
    InitBitmapInfo(&bmi, sizeof(bmi), sizImage.cx, sizImage.cy, 32);

    HRESULT hr(E_OUTOFMEMORY);

	auto Deleter = [](BYTE* pb) { if (pb) ::CoTaskMemFree(pb); };
	std::unique_ptr<BYTE, decltype(Deleter)> pvBits(reinterpret_cast<BYTE*>(::CoTaskMemAlloc(bmi.bmiHeader.biWidth * 4 * bmi.bmiHeader.biHeight)), Deleter);

	if (pvBits)
    {
		::ZeroMemory(pvBits.get(), bmi.bmiHeader.biWidth * 4 * bmi.bmiHeader.biHeight);
        hr = E_UNEXPECTED;
        if (::GetDIBits(hdc, hbmp, 0, bmi.bmiHeader.biHeight, pvBits.get(), &bmi, DIB_RGB_COLORS) == bmi.bmiHeader.biHeight)
        {
            ULONG cxDelta = cxRow - bmi.bmiHeader.biWidth;
			ARGB *pargbMask(reinterpret_cast<ARGB*>(pvBits.get()));

            for (ULONG y(bmi.bmiHeader.biHeight); y; --y)
            {
                for (ULONG x(bmi.bmiHeader.biWidth); x; --x)
                {
                    if (*pargbMask++)
                        *pargb++ = 0;
                    else
                        *pargb++ |= 0xFF000000;
                }
                pargb += cxDelta;
            }
            hr = S_OK;
        }
    }
    return hr;
}

bool CCoPE32Viewer::HasAlpha(__in ARGB *pargb, __in SIZE& sizImage, __in int cxRow)
{
    ULONG cxDelta(cxRow - sizImage.cx);
    for (ULONG y(sizImage.cy); y; --y)
    {
        for (ULONG x(sizImage.cx); x; --x)
        {
            if (*pargb++ & 0xFF000000)
                return true;
        }
        pargb += cxDelta;
    }
    return false;
}

HRESULT CCoPE32Viewer::AddIconToMenuItemGDI(__in HICON hIcon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp)
{
	if (!hIcon)
	{
		if (phbmp)
			*phbmp = nullptr;
		return E_FAIL;
	}

	RECT rect;

	rect.right = ::GetSystemMetrics(SM_CXMENUCHECK);
	rect.bottom = ::GetSystemMetrics(SM_CYMENUCHECK);

	rect.left = rect.top  = 0;


	HWND desktop(::GetDesktopWindow());
	if (!desktop)
	{
		if (fAutoDestroy)
			::DestroyIcon(hIcon);
		if (phbmp)
			*phbmp = nullptr;
		return E_FAIL;
	}

	HDC screen_dev(::GetDC(desktop));
	if (!screen_dev)
	{
		if (fAutoDestroy)
			::DestroyIcon(hIcon);
		if (phbmp)
			*phbmp = nullptr;
		return E_FAIL;
	}

	// Create a compatible DC
	HDC dst_hdc(::CreateCompatibleDC(screen_dev));
	if (!dst_hdc)
	{
		if (fAutoDestroy)
			::DestroyIcon(hIcon);
		if (phbmp)
			*phbmp = nullptr;
		::ReleaseDC(desktop, screen_dev);
		return E_FAIL;
	}

	// Create a new bitmap of icon size
	HBITMAP bmp(::CreateCompatibleBitmap(screen_dev, rect.right, rect.bottom));
	if (bmp == NULL)
	{
		if (fAutoDestroy)
			::DestroyIcon(hIcon);
		if (phbmp)
			*phbmp = nullptr;
		::DeleteDC(dst_hdc);
		::ReleaseDC(desktop, screen_dev);
		return E_FAIL;
	}

	// Select it into the compatible DC
	HBITMAP old_dst_bmp(reinterpret_cast<HBITMAP>(::SelectObject(dst_hdc, bmp)));
	if (!old_dst_bmp)
	{
		if (fAutoDestroy)
			::DestroyIcon(hIcon);
		if (phbmp)
			*phbmp = nullptr;
		::DeleteDC(dst_hdc);
		::ReleaseDC(desktop, screen_dev);
		::DeleteObject(bmp);
		return E_FAIL;
	}

	// Fill the background of the compatible DC with the given colour
	::SetBkColor(dst_hdc, RGB(255, 255, 255));
	::ExtTextOut(dst_hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);

	// Draw the icon into the compatible DC
	::DrawIconEx(dst_hdc, 0, 0, hIcon, rect.right, rect.bottom, 0, NULL, DI_NORMAL);

	// Restore settings
	::SelectObject(dst_hdc, old_dst_bmp);
	::DeleteDC(dst_hdc);
	::ReleaseDC(desktop, screen_dev);
	if (fAutoDestroy)
		::DestroyIcon(hIcon);
	if (phbmp)
		*phbmp = bmp;
	return S_OK;
}

HRESULT CCoPE32Viewer::AddIconToMenuItem(__in IWICImagingFactory *pFactory, __in HICON hicon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp)
{
    HBITMAP hbmp(nullptr);

    CComPtr<IWICBitmap> pBitmap;
	HRESULT hr(pFactory ->CreateBitmapFromHICON(hicon, &pBitmap.p));
    if (SUCCEEDED(hr))
    {
        CComPtr<IWICFormatConverter> pConverter;
        hr = pFactory ->CreateFormatConverter(&pConverter.p);
        if (SUCCEEDED(hr))
        {
            hr = pConverter ->Initialize(pBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);
            if (SUCCEEDED(hr))
            {
                UINT cx, cy;
                hr = pConverter ->GetSize(&cx, &cy);
                if (SUCCEEDED(hr))
                {
                    const SIZE sizIcon = { (int)cx, -(int)cy };
                    BYTE *pbBuffer;
                    hr = Create32BitHBITMAP(nullptr, &sizIcon, reinterpret_cast<void **>(&pbBuffer), &hbmp);
                    if (SUCCEEDED(hr))
                    {
                        const UINT cbStride = cx * sizeof(ARGB);
                        const UINT cbBuffer = cy * cbStride;
                        hr = pConverter ->CopyPixels(nullptr, cbStride, cbBuffer, pbBuffer);
                    }
                }
            }
        }
    }

    if (FAILED(hr))
    {
        ::DeleteObject(hbmp);
        hbmp = nullptr;
    }

    if (fAutoDestroy)
        DestroyIcon(hicon);

    if (phbmp)
        *phbmp = hbmp;

    return hr;
}

HRESULT CCoPE32Viewer::AddIconToMenuItemNoWIC(__in HICON hicon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp)
{
    HRESULT hr(E_OUTOFMEMORY);
    HBITMAP hbmp(nullptr);

    SIZE sizIcon;
    sizIcon.cx = ::GetSystemMetrics(SM_CXSMICON);
    sizIcon.cy = ::GetSystemMetrics(SM_CYSMICON);

    RECT rcIcon;
    ::SetRect(&rcIcon, 0, 0, sizIcon.cx, sizIcon.cy);

    HDC hdcDest(::CreateCompatibleDC(nullptr));
    if (hdcDest)
    {
        hr = Create32BitHBITMAP(hdcDest, &sizIcon, nullptr, &hbmp);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;

            HBITMAP hbmpOld(reinterpret_cast<HBITMAP>(::SelectObject(hdcDest, hbmp)));
            if (hbmpOld)
            {
                BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
                BP_PAINTPARAMS paintParams = {0};
                paintParams.cbSize = sizeof(paintParams);
                paintParams.dwFlags = BPPF_ERASE;
                paintParams.pBlendFunction = &bfAlpha;

                HDC hdcBuffer;
                HPAINTBUFFER hPaintBuffer(::BeginBufferedPaint(hdcDest, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer));
                if (hPaintBuffer)
                {
                    if (::DrawIconEx(hdcBuffer, 0, 0, hicon, sizIcon.cx, sizIcon.cy, 0, nullptr, DI_NORMAL))
                        // If icon did not have an alpha channel, we need to convert buffer to PARGB.
                        hr = ConvertBufferToPARGB32(hPaintBuffer, hdcDest, hicon, sizIcon);

                    // This will write the buffer contents to the destination bitmap.
                    ::EndBufferedPaint(hPaintBuffer, TRUE);
                }
                ::SelectObject(hdcDest, hbmpOld);
            }
        }
        ::DeleteDC(hdcDest);
    }

    if (FAILED(hr))
    {
        ::DeleteObject(hbmp);
        hbmp = nullptr;
    }

    if (fAutoDestroy)
        ::DestroyIcon(hicon);

    if (phbmp)
        *phbmp = hbmp;

    return hr;
}

STDMETHODIMP CCoPE32Viewer::GetCommandString(__in  UINT_PTR idCmd, __in  UINT uType, __reserved  UINT *pReserved, __out_awcount(!(uType & GCS_UNICODE), cchMax)  LPSTR pszName, __in  UINT cchMax)
{
	HRESULT hr(E_INVALIDARG);

    if(!idCmd || idCmd != mf_nCmdID)
        return hr;

	switch(uType)
	{
	case GCS_HELPTEXTW:
		{
			CComBSTR pwszText = LoadStringEx(mf_hInstance, IDS_MENUITEMHELPTEXT, ::GetUserDefaultLangID());
			if (!pwszText.m_str)
				pwszText = L"Executable dependency list";
			else
				::lstrcpynW(reinterpret_cast<LPWSTR>(pszName), pwszText.m_str, cchMax);
			hr = S_OK;
		}
		break;
	case GCS_HELPTEXTA:
		{
			auto Deleter = [](char* pch) { if (pch) ::CoTaskMemFree(pch); };
			std::unique_ptr<char, decltype(Deleter)> lptszText(LoadAnsiStringEx(mf_hInstance, IDS_MENUITEMHELPTEXT, ::GetUserDefaultLangID()), Deleter);

			if (!lptszText)
				::StringCchCopyA(pszName, cchMax, "Executable dependency list");
			else
				::StringCchCopyA(pszName, cchMax, lptszText.get());
			hr = S_OK;
		}
		break;
	case GCS_VERBA:
		{
			::StringCchCopyA(pszName, cchMax, mf_szVerbA);
			hr = S_OK;
		}
		break;
	case GCS_VERBW:
		{
			::StringCchCopyW(reinterpret_cast<LPWSTR>(pszName), cchMax, mf_szVerbW);
			hr = S_OK;
		}
		break;
    default:
            hr = S_OK;
            break; 
	}

	return S_OK;
}

void CALLBACK CCoPE32Viewer::PropertiesShowWorker(__inout PTP_CALLBACK_INSTANCE Instance,  __inout_opt  PWORKER_INIT_PARAMS pInitParams)
{
	::InterlockedIncrement(&g_nCount);

	if (pInitParams && (sizeof(WORKER_INIT_PARAMS) == pInitParams ->dwStructSize))
	{
		CComBSTR FileName;
		FileName.Attach(pInitParams ->pszFileName);

		POINT pt(pInitParams ->ptInvokePt);
		HWND hParent(pInitParams ->hParentWnd);

		::SetEvent(pInitParams ->hEvent);

		CCOMInitializer COMInit;
		if (SUCCEEDED(COMInit.InitializationResult()))
		{
			CPE32Decoder PE32Decoder(FileName.m_str);
			if (SUCCEEDED(PE32Decoder.InitResult()))
			{
				::CallbackMayRunLong(Instance);

				CPE32PropsView PE32ViewDlg(&PE32Decoder, pt, hParent);
				PE32ViewDlg.Create(hParent);				
				PE32ViewDlg.ShowWindow(SW_SHOWNORMAL);

				MSG msg;
				while (::GetMessage(&msg, 0, 0, 0))
				{
					if (!::IsDialogMessageW(PE32ViewDlg.m_hWnd, &msg))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
			}
		}

		FileName.Empty();
	}
	::InterlockedDecrement(&g_nCount);
}

STDMETHODIMP CCoPE32Viewer::InvokeCommand(__in  CMINVOKECOMMANDINFO *pici)
{
	if (HIWORD(pici ->lpVerb))
		return E_INVALIDARG;

	if (!LOWORD(pici ->lpVerb))
	{
		WORKER_INIT_PARAMS wip = { 0 };
		if (sizeof(CMINVOKECOMMANDINFOEX) == pici ->cbSize)
		{
			LPCMINVOKECOMMANDINFOEX piciex(reinterpret_cast<LPCMINVOKECOMMANDINFOEX>(pici));
			mf_ptPoint = piciex ->ptInvoke;
		}
		else
		{
			mf_ptPoint.x = -1;
			mf_ptPoint.y = -1;
		}

		CXHandle Event(::CreateEventW(nullptr, TRUE, FALSE, nullptr));
		if (Event.IsValidHandle())
		{
			wip.dwStructSize = sizeof(WORKER_INIT_PARAMS);
			wip.hEvent = Event.Handle();
			CComBSTR FileName = mf_szFileName;

			if (FileName.m_str)
			{
				wip.pszFileName = FileName.Detach();
				wip.ptInvokePt = mf_ptPoint;
				wip.hParentWnd = pici ->hwnd;

				if (::TrySubmitThreadpoolCallback(reinterpret_cast<PTP_SIMPLE_CALLBACK>(PropertiesShowWorker), &wip, nullptr))
					::WaitForSingleObject(Event.Handle(), INFINITE);			
			}
			else
				return E_INVALIDARG;
		}
		else
			return E_INVALIDARG;
		return S_OK;
	}
	else
		return E_INVALIDARG;
}

STDMETHODIMP CCoPE32Viewer::InitializeTheme(__in HWND hwndTheme)
{
    HRESULT hr(E_FAIL);
	mf_hwndTheme = hwndTheme;
    mf_hTheme = ::OpenThemeData(mf_hwndTheme, VSCLASS_MENU);
    if (mf_hTheme)
    {
        ::GetThemePartSize(mf_hTheme, nullptr, MENU_POPUPCHECK, 0, nullptr, TS_TRUE, &mf_sizePopupCheck);
        ::GetThemePartSize(mf_hTheme, nullptr, MENU_POPUPSEPARATOR, 0, nullptr, TS_TRUE, &mf_sizePopupSeparator);
		::GetThemeInt(mf_hTheme, MENU_POPUPITEM, 0, TMT_BORDERSIZE, &mf_iPopupBorderSize); 
        ::GetThemeInt(mf_hTheme, MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &mf_iPopupBgBorderSize); 
		::GetThemeMargins(mf_hTheme, nullptr, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, nullptr, &mf_marPopupCheck); 
        ::GetThemeMargins(mf_hTheme, nullptr, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, nullptr, &mf_marPopupCheckBackground);
        ::GetThemeMargins(mf_hTheme, nullptr, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, nullptr, &mf_marPopupItem);

		::GetThemeColor(mf_hTheme, MENU_POPUPITEM, MPI_DISABLED, TMT_COLOR, &mf_crItemDisabled);
		::GetThemeColor(mf_hTheme, MENU_POPUPITEM, MPI_DISABLEDHOT, TMT_COLOR, &mf_crItemDisabledHot);
		::GetThemeColor(mf_hTheme, MENU_POPUPITEM, MPI_HOT, TMT_COLOR, &mf_crItemHot);
		::GetThemeColor(mf_hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_COLOR, &mf_crItemNormal);

        mf_marPopupAccelerator = mf_marPopupItem;
        mf_marPopupAccelerator.cxLeftWidth = mf_marPopupAccelerator.cxRightWidth = 0;

        // Popup text margins
        MARGINS margins = mf_marPopupItem;
        margins.cxRightWidth = mf_iPopupBorderSize;
        margins.cxLeftWidth = mf_iPopupBgBorderSize;
        mf_marPopupText = margins;
        
        mf_cyMarCheckBackground = mf_marPopupCheckBackground.cyTopHeight + mf_marPopupCheckBackground.cyBottomHeight;
        hr = S_OK;
    }
    return hr;
}

void CCoPE32Viewer::Flush(void)
{
    if (mf_hTheme)
    {
        ::CloseThemeData(mf_hTheme);
        mf_hTheme = nullptr;
    }
}

void CCoPE32Viewer::ToMeasureSize(const SIZE *psizeDraw, const MARGINS *pmargins, __out LPSIZE psizeMeasure)
{
    psizeMeasure->cx = psizeDraw->cx + pmargins->cxLeftWidth + pmargins->cxRightWidth;
    psizeMeasure->cy = psizeDraw->cy + pmargins->cyTopHeight + pmargins->cyBottomHeight;
}

void CCoPE32Viewer::MeasureMenuItem(__in HWND hwndTheme, __inout MENUITEM *pmi, __inout MEASUREITEMSTRUCT *pmis)
{
	Flush();
	InitializeTheme(hwndTheme);

    int cxTotal(0);
    int cyMax(0);
	SIZE sizeDraw = { 0 };

    ::ZeroMemory(pmi->pmid->rgPopupSize, sizeof(pmi->pmid->rgPopupSize));
    MENUITEMINFO *pmii = &pmi->mii;

    // Size the check rectangle.
    sizeDraw = mf_sizePopupCheck;
    sizeDraw.cy += mf_marPopupCheckBackground.cyTopHeight + mf_marPopupCheckBackground.cyBottomHeight;
    
    ToMeasureSize(&sizeDraw, &mf_marPopupCheck, &pmi ->pmid ->rgPopupSize[POPUP_CHECK]);
    cxTotal += pmi ->pmid ->rgPopupSize[POPUP_CHECK].cx;
	cyMax += pmi ->pmid ->rgPopupSize[POPUP_CHECK].cy;

    if (pmii ->fType & MFT_SEPARATOR)
    {
        // Size the separator, using the minimum width.
        sizeDraw = mf_sizePopupCheck;
        sizeDraw.cy = mf_sizePopupSeparator.cy;
        
        ToMeasureSize(&sizeDraw, &mf_marPopupItem, &pmi->pmid ->rgPopupSize[POPUP_SEPARATOR]);
    }
    else
    {
        // Add check background horizontal padding.
        cxTotal += mf_marPopupCheckBackground.cxLeftWidth + mf_marPopupCheckBackground.cxRightWidth;

        if (pmii->cch)
        {
            HDC hdc(::GetDC(hwndTheme));
            if (hdc)
            {
                // Size the text subitem rectangle.
                RECT rcText = { 0 };
                if (SUCCEEDED(::GetThemeTextExtent(mf_hTheme, hdc, MENU_POPUPITEM, 0, pmi ->szItemText, pmii ->cch, DT_LEFT | DT_SINGLELINE, nullptr, &rcText)))
				{
					sizeDraw.cx = rcText.right + ::GetSystemMetrics(SM_CXICON);
					sizeDraw.cy = rcText.bottom;

					if (sizeDraw.cy < ::GetSystemMetrics(SM_CYICON))
						sizeDraw.cy = ::GetSystemMetrics(SM_CYICON);

					ToMeasureSize(&sizeDraw, &mf_marPopupText, &pmi->pmid ->rgPopupSize[POPUP_TEXT]);
					cxTotal += sizeDraw.cx;//pmi->pmid->rgPopupSize[POPUP_TEXT].cx;
					cyMax += sizeDraw.cy;//pmi->pmid->rgPopupSize[POPUP_TEXT].cy;
				}
				
                ::ReleaseDC(hwndTheme, hdc);
            }                
        }

        // Account for selection margin padding.
        cxTotal += mf_marPopupItem.cxLeftWidth + mf_marPopupItem.cxRightWidth;
		cyMax += mf_marPopupItem.cyTopHeight + mf_marPopupItem.cyBottomHeight;
    }
    
    // Calculate maximum menu item height.
    if (pmii->fType & MFT_SEPARATOR)
    {
        cyMax = pmi->pmid->rgPopupSize[POPUP_SEPARATOR].cy;
    }
    else
    {
        for (UINT i(0); i < POPUP_MAX; ++i)
            cyMax = max(cyMax, pmi->pmid->rgPopupSize[i].cy);
    }

    // Return the composite sizes.
    pmis->itemWidth = cxTotal;
    pmis->itemHeight = cyMax;
}

void CCoPE32Viewer::ToDrawRect(__in LPCRECT prcMeasure, __in const MARGINS *pmargins, __out LPRECT prcDraw)
{
	// Convert the measure rect to a drawing rect.
        ::SetRect(prcDraw, prcMeasure->left    + pmargins->cxLeftWidth, prcMeasure->top     + pmargins->cyTopHeight, prcMeasure->right   - pmargins->cxRightWidth, prcMeasure->bottom  - pmargins->cyBottomHeight);
}

void CCoPE32Viewer::LayoutMenuItem(__in MENUITEM *pmi, __in DRAWITEMSTRUCT *pdis, __out DRAWITEMMETRICS *pdim)
{
    const RECT *prcItem(&pdis ->rcItem);
    const int cyItem(RectHeight(pdis ->rcItem));
    const SIZE *prgsize(pmi ->pmid ->rgPopupSize);
    const bool  fIsSeparator((pmi ->mii.fType & MFT_SEPARATOR) != 0);
    int x(prcItem ->left + mf_marPopupItem.cxLeftWidth);							// Left gutter margin
    const int y(prcItem ->top);
    RECT rcMeasure;

    for (UINT i(0); i < POPUP_MAX; ++i)
    {
		int cx;
        int cy;
        if (prgsize[i].cx)
        {
            switch (i)
            {
            case POPUP_CHECK:
                // Add left padding for the check background rectangle.
                x += mf_marPopupCheckBackground.cxLeftWidth;

                // Right-align the check/bitmap in the column.            
                cx =  prgsize[i].cx;
                cy =  prgsize[i].cy - mf_cyMarCheckBackground;
                break;

            default:
                cx = prgsize[i].cx;
                cy = prgsize[i].cy;
                break;
            }

            // Position and vertically center the subitem.
            ::SetRect(&rcMeasure, x, y, x + cx, y + cy);
            
			if (POPUP_CHECK == i)
                ToDrawRect(&rcMeasure, &mf_marPopupCheck, &pdim->rgrc[i]);
            else
                ToDrawRect(&rcMeasure, &mf_marPopupText, &pdim->rgrc[i]);

			::OffsetRect(&pdim->rgrc[i], 0, (cyItem - cy) / 2);

            // Add right padding for the check background rectangle.
            if (POPUP_CHECK == i)
                x += mf_marPopupCheckBackground.cxRightWidth;

            // Move to the next subitem.
            x += cx;
        }
    }

    // Calculate the check background draw size.
	SIZE sizeDraw = { 0 };
    sizeDraw.cx = prgsize[POPUP_CHECK].cx;
    sizeDraw.cy = prgsize[POPUP_CHECK].cy - mf_cyMarCheckBackground;

    // Calculate the check background measure size.
	SIZE sizeMeasure = { 0 };
    ToMeasureSize(&sizeDraw, &mf_marPopupCheckBackground, &sizeMeasure);

    // Lay out the check background rectangle.
    x = prcItem ->left + mf_marPopupItem.cxLeftWidth;

    ::SetRect(&rcMeasure, x, y, x + sizeMeasure.cx, y + sizeMeasure.cy);
    ToDrawRect(&rcMeasure, &mf_marPopupCheckBackground, &pdim ->rcCheckBackground);
    ::OffsetRect(&pdim->rcCheckBackground, 0, (cyItem - sizeMeasure.cy) / 2);

    // Lay out gutter rectangle.
    x = prcItem ->left;

    sizeDraw.cx = prgsize[POPUP_CHECK].cx;
    ToMeasureSize(&sizeDraw, &mf_marPopupCheckBackground, &sizeMeasure);

    ::SetRect(&pdim ->rcGutter, x, y, x + mf_marPopupItem.cxLeftWidth + sizeMeasure.cx, y + cyItem);

    if (fIsSeparator)
    {
        // Lay out the separator rectangle.
        x = pdim ->rcGutter.right + mf_marPopupItem.cxLeftWidth;

        ::SetRect(&rcMeasure, x, y, prcItem ->right - mf_marPopupItem.cxRightWidth, y + prgsize[POPUP_SEPARATOR].cy);
        ToDrawRect(&rcMeasure, &mf_marPopupItem, &pdim ->rgrc[POPUP_SEPARATOR]);
        ::OffsetRect(&pdim ->rgrc[POPUP_SEPARATOR], 0, (cyItem - prgsize[POPUP_SEPARATOR].cy) / 2);
    }
    else
    {
        // Lay out selection rectangle.
        x = prcItem ->left + mf_marPopupItem.cxLeftWidth;
		::SetRect(&pdim ->rcSelection, x, y, prcItem ->right - mf_marPopupItem.cxRightWidth, y + cyItem);   
    }
}

POPUPITEMSTATES CCoPE32Viewer::ToItemStateId(__in UINT uItemState)
{
    const bool fDisabled((uItemState & (ODS_INACTIVE | ODS_DISABLED)) != 0);
    const bool fHot((uItemState & (ODS_HOTLIGHT | ODS_SELECTED)) != 0);
    POPUPITEMSTATES iState;

    if (fDisabled)
        iState = (fHot ? MPI_DISABLEDHOT : MPI_DISABLED);
    else if (fHot)
        iState = MPI_HOT;
    else
        iState= MPI_NORMAL;
    
    return iState;
}

POPUPCHECKBACKGROUNDSTATES CCoPE32Viewer::ToCheckBackgroundStateId(__in int iStateId)
{
    POPUPCHECKBACKGROUNDSTATES iStateIdCheckBackground;
    // Determine the check background state.
    if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
        iStateIdCheckBackground = MCB_DISABLED;
    else
        iStateIdCheckBackground = MCB_NORMAL;

    return iStateIdCheckBackground;
}

POPUPCHECKSTATES CCoPE32Viewer::ToCheckStateId(__in UINT fType, __in int iStateId)
{
    POPUPCHECKSTATES iStateIdCheck;        
    if (fType & MFT_RADIOCHECK)
    {
        if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
            iStateIdCheck = MC_BULLETDISABLED;
        else
            iStateIdCheck = MC_BULLETNORMAL;
    }
    else
    {
        if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
            iStateIdCheck = MC_CHECKMARKDISABLED;
        else
            iStateIdCheck = MC_CHECKMARKNORMAL;
    }
    return iStateIdCheck;
}        

void CCoPE32Viewer::ThemedDrawMenuItem(__in HWND hwndTheme, __in MENUITEM *pmi, __in DRAWITEMSTRUCT *pdis)
{
    POPUPITEMSTATES iStateId(ToItemStateId(pdis ->itemState));
	DRAWITEMMETRICS dim = { 0 };
    LayoutMenuItem(pmi, pdis, &dim);

    if (::IsThemeBackgroundPartiallyTransparent(mf_hTheme,  MENU_POPUPITEM, iStateId))
		::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPBACKGROUND, 0, &pdis ->rcItem, nullptr);
    ::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPGUTTER, 0, &dim.rcGutter, nullptr);             
    if (pmi->mii.fType & MFT_SEPARATOR)
        ::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPSEPARATOR, 0, &dim.rgrc[POPUP_SEPARATOR], nullptr);
    else
    {
        // Item selection
        ::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPITEM, iStateId, &dim.rcSelection, nullptr);

        // Draw the checkbox if necessary.
        if (pmi->mii.fState & MFS_CHECKED)
        {
            ::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPCHECKBACKGROUND, ToCheckBackgroundStateId(iStateId), &dim.rcCheckBackground, nullptr);
			::DrawThemeBackground(mf_hTheme, pdis ->hDC, MENU_POPUPCHECK, ToCheckStateId(pmi ->mii.fType, iStateId), &dim.rgrc[POPUP_CHECK], nullptr);
        }

		HIMAGELIST himlIcons(::ImageList_Create(::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 0));
		if (himlIcons)
		{
			HICON hIcon;
			if (SUCCEEDED(::LoadIconMetric(mf_hInstance, MAKEINTRESOURCE(IDI_DLGLOGOICON), LIM_LARGE, &hIcon)))
			{
				int nIconIndex;
				if (-1 != (nIconIndex = ::ImageList_AddIcon(himlIcons, hIcon)))
					::DrawThemeIcon(mf_hTheme, pdis ->hDC, MENU_POPUPITEM, iStateId, &dim.rgrc[POPUP_MAX], himlIcons, 0); //
			}
			::ImageList_Destroy(himlIcons);
			::DestroyIcon(hIcon);
		}

        // Draw the text.
        ULONG uAccel((pdis->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
        ::DrawThemeText(mf_hTheme, pdis ->hDC, MENU_POPUPITEM, iStateId, pmi->mii.dwTypeData, pmi->mii.cch, DT_SINGLELINE | DT_LEFT | uAccel, 0, &dim.rgrc[POPUP_TEXT]);
    }
	Flush();
}

BOOL CCoPE32Viewer::GetMenuItem(__in ULONG_PTR itemData, __out_bcount(sizeof(MENUITEM)) MENUITEM *pItem)
{
    BOOL fRet(FALSE);

    if (itemData)
    {
        pItem->pmid = reinterpret_cast<MENUITEMDATA *>(itemData);
        pItem->szItemText[0] = 0;
        pItem->mii.cbSize = sizeof(pItem->mii);
        pItem->mii.fMask = MIIM_CHECKMARKS | MIIM_BITMAP | MIIM_STRING | MIIM_SUBMENU | MIIM_STATE | MIIM_FTYPE | MIIM_ID;
        pItem->mii.dwTypeData = pItem->szItemText;
        pItem->mii.cch = ARRAYSIZE(pItem->szItemText);
        fRet = ::GetMenuItemInfoW(pItem->pmid->hMenu, pItem->pmid->ID, TRUE, &pItem->mii);
    }

    return fRet;
}

//	Message handler
STDMETHODIMP CCoPE32Viewer::ProcessMenuMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam, __out LRESULT *plResult)
{
	HRESULT hResult(S_OK);
	LRESULT lResult(0);

    UINT fuFlags;
	HMENU hMenu;
	LPMEASUREITEMSTRUCT pMIS;
	LPDRAWITEMSTRUCT pDIS;
	MENUITEM menuItem;

	switch(uMsg)
	{
	case WM_MENUCHAR:
		lResult = MNC_IGNORE;
		break;
	case WM_INITMENUPOPUP:
		hMenu = reinterpret_cast<HMENU>(wParam);
		fuFlags = MF_BYCOMMAND | MF_ENABLED;
		::EnableMenuItem(hMenu, mf_nCmdID, fuFlags);
		break;
	case WM_DRAWITEM:
		pDIS = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
	    if (mf_nCmdID != pDIS ->itemID )
			break;
		if (!::IsRectEmpty(&pDIS ->rcItem))
		{
			int iSaveDC(::SaveDC(pDIS ->hDC));

			if (GetMenuItem(pDIS ->itemData, &menuItem))
			{
				ThemedDrawMenuItem(nullptr, &menuItem, pDIS);
				lResult = 1;
			}

			::RestoreDC(pDIS ->hDC, iSaveDC);
		}
		break;
	case WM_MEASUREITEM:
		pMIS = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
	    if (mf_nCmdID != pMIS ->itemID )
			break;
		if (GetMenuItem(pMIS ->itemData, &menuItem))
		{
			MeasureMenuItem(nullptr, &menuItem, pMIS);
			lResult = 1;
		}
	    else
			pMIS ->itemWidth = pMIS ->itemHeight = 0;
		break;
	}
	if (plResult)
		*plResult = lResult;
	return hResult;
}

//	IContextMenu2
STDMETHODIMP CCoPE32Viewer::HandleMenuMsg(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam)
{
	return ProcessMenuMessage(uMsg, wParam, lParam, nullptr);
}

//	IContextMenu3
STDMETHODIMP CCoPE32Viewer::HandleMenuMsg2(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam, __out_opt LRESULT *plResult)
{
	return ProcessMenuMessage(uMsg, wParam, lParam, plResult);
}

  