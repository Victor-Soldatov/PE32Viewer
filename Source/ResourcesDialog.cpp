#include "stdafx.h"
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"
#include <msxml6.h>
#include <propvarutil.h>
#include <commctrl.h>

#include <wincodec.h>
#pragma comment(lib, "Windowscodecs.lib")

#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "Version.lib")

#define STRSAFE_LIB
#include <strsafe.h>
#include <memory>

#define TMP_BUFFER_LENGTH					128
#define TMP_LARGE_BUFFER_LENGTH				256
#define TMP_BUFFER_LENGTH_MAX				4096
#define MAX_NAMES							64
#define MENU_BITMAP_MAX						5

const __wchar_t lpwszTypeLibDlgProp[] = L"TypeLibDlgProp_ClassRef";
const __wchar_t lpwszResourcesDlgProp[] = L"UnknownResourcesDlgProp_ClassRef";
const __wchar_t lpwszViewSubjResDlgProp[] = L"UnkResViewDlgProp_ViewToolSubj";

const __wchar_t lpwszViewSubjResDlgPropHBmp[] = L"UnkResViewDlgProp_ViewToolSubjBitmapHandle";
const __wchar_t lpwszViewSubjResDlgPropHIcon[] = L"UnkResViewDlgProp_ViewToolSubjIconHandle";

const __wchar_t lpwszViewResToolDlgSmallIconProp[] = L"UnkResViewDlgSmallIconProp_ViewToolDlgIconHandle";
const __wchar_t lpwszViewResToolDlgLargeIconProp[] = L"UnkResViewDlgLargeIconProp_ViewToolDlgLargeIconHandle";

const __wchar_t lpwszTypeLibItemDelimiter[] = L", ";
const __wchar_t lpwszTypeLibItemRef[] = L"* ";
const __wchar_t lpwszTypeLibSpace[] = L" ";

struct tm * gmtime(__in time_t *timer, __out struct tm *p_out_time);
struct tm * gmtime_t(__in __time32_t timer, __out struct tm *p_out_time);

extern HINSTANCE hDLLInstance;

const __wchar_t* pwszSubString[] = {
	L"Comments",
	L"InternalName",
	L"ProductName",
	L"CompanyName",
	L"LegalCopyright",
	L"ProductVersion",
	L"FileDescription",
	L"LegalTrademarks",
	L"PrivateBuild",
	L"FileVersion",
	L"OriginalFilename",
	L"SpecialBuild"
};

#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
   BITMAPINFOHEADER icHeader;
   RGBQUAD icColors[1];
   BYTE icXOR[1];
   BYTE icAND[1];
} 
ICONIMAGE, *LPICONIMAGE;

typedef struct
{
   BYTE   bWidth;					// Width, in pixels, of the image
   BYTE   bHeight;					// Height, in pixels, of the image
   BYTE   bColorCount;				// Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;				// Reserved
   WORD   wPlanes;					// Color Planes
   WORD   wBitCount;				// Bits per pixel
   DWORD  dwBytesInRes;				// how many bytes in this resource?
   WORD   nID;						// the ID
} 
GRPICONDIRENTRY, *LPGRPICONDIRENTRY;
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 2 )
typedef struct 
{
   WORD            idReserved;		// Reserved (must be 0)
   WORD            idType;			// Resource type (1 for icons)
   WORD            idCount;			// How many images?
   GRPICONDIRENTRY   idEntries[1];	// The entries for each image
} 
GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )

typedef struct __tagVIEWTOOLINFO
{
	DWORD cbSize;
	CPE32PropsView* pMainDlg;
	int nItemIndex;
	UNK_RES_ITEM ResItem;
}
VIEWTOOLINFO, *PVIEWTOOLINFO;

#define VSEDIT						L"EditStyle"

#define WIDECHAR_MASK				0x0FFFFFFFEUL
#define HEX_VIEW_COLUMNS			10
#define HEX_VIEW_OFFSET_WIDTH		8
#define HEX_VIEW_BYTE_WIDTH			2
#define HEX_VIEW_SYM_WIDTH			8
#define MEM_CHUNK_SIZE				8UL

typedef struct __tagBYTE_VIEW
{
	__wchar_t wszByteView[3];
}
BYTE_VIEW, *PBYTE_VIEW;

typedef struct __tagMEM_CHUNK_VIEW
{
	__wchar_t Offset[HEX_VIEW_OFFSET_WIDTH + 1];
	BYTE_VIEW Bytes[MEM_CHUNK_SIZE];
	__wchar_t Symbols[MEM_CHUNK_SIZE + 1];
}
MEM_CHUNK_VIEW, *PMEM_CHUNK_VIEW;

typedef LANGANDCODEPAGE *PLANGANDCODEPAGE;

INT_PTR CALLBACK CPE32PropsView::ChildViewAsIconDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hDlgSmIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
			if (!hDlgSmIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), LIM_SMALL, &hDlgSmIcon);
			if (!hDlgSmIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hDlgSmIcon);
			if (!hDlgSmIcon)
				hDlgSmIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO));

			if (hDlgSmIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgSmallIconProp, hDlgSmIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
				else
				{
					::DestroyIcon(hDlgSmIcon);
					hDlgSmIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			HICON hDlgIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)));
			if (!hDlgIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), LIM_LARGE, &hDlgIcon);
			if (!hDlgIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO), ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), &hDlgIcon);
			if (!hDlgIcon)
				hDlgIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_ICO));

			if (hDlgIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgLargeIconProp, hDlgIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hDlgIcon));
				else
				{
					::DestroyIcon(hDlgIcon);
					hDlgIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(lParam));
			if (pVTI && (sizeof(VIEWTOOLINFO) == pVTI ->cbSize))
			{
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
				HWND hPicView(::GetDlgItem(hDlg, IDC_PICVIEW));
				HWND hButton(::GetDlgItem(hDlg, IDOK));

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hPicView, VSCLASS_EXPLORER, nullptr);
					::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
				}

				DWORD dwDlgBaseWidthUnit(LOWORD(::GetDialogBaseUnits()));
				DWORD dwDlgBaseHeightUnit(HIWORD(::GetDialogBaseUnits()));

				CResString Caption(IDS_ICON_DLG_CAPTION);
				::SetWindowTextW(hDlg, Caption.m_bszResString.m_str);

				//	Adjust edit control size/position
				RECT OkButtonRect;
				::GetWindowRect(hButton, &OkButtonRect);
				::MapWindowPoints(HWND_DESKTOP, hDlg, reinterpret_cast<LPPOINT>(&OkButtonRect), sizeof(RECT) / sizeof(POINT));

				RECT DlgClientRect;
				::GetClientRect(hDlg, &DlgClientRect);
				DlgClientRect.right--;
				DlgClientRect.bottom--;

				::MoveWindow(hPicView, 1, 1, DlgClientRect.right - 1, OkButtonRect.top - dwDlgBaseHeightUnit, FALSE);

				if (pVTI ->ResItem.Desc.dwResItemDataSize && pVTI ->ResItem.Desc.lpResItemDataPtr)
				{
					HICON hIcon(::CreateIconFromResource(pVTI ->ResItem.Desc.lpResItemDataPtr, pVTI ->ResItem.Desc.dwResItemDataSize, TRUE, 0x00030000));
					if (hIcon)
					{
						if (::SetPropW(hDlg, lpwszViewSubjResDlgPropHIcon, hIcon))
							::SendMessageW(hPicView, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon));
						else
							::DeleteObject(hIcon);
					}
				}
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
                ::EndDialog(hDlg, 1);
                return TRUE; 
        } 
		break;
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case SC_CLOSE:
				::EndDialog(hDlg, 1);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			HICON hIcon(reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgSmallIconProp)));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon = reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgLargeIconProp));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon =reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewSubjResDlgPropHIcon));
			if (hIcon)
				::DeleteObject(hIcon);
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK CPE32PropsView::ChildViewAsBitmapDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hDlgSmIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
			if (!hDlgSmIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), LIM_SMALL, &hDlgSmIcon);
			if (!hDlgSmIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hDlgSmIcon);
			if (!hDlgSmIcon)
				hDlgSmIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP));

			if (hDlgSmIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgSmallIconProp, hDlgSmIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
				else
				{
					::DestroyIcon(hDlgSmIcon);
					hDlgSmIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			HICON hDlgIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)));
			if (!hDlgIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), LIM_LARGE, &hDlgIcon);
			if (!hDlgIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP), ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), &hDlgIcon);
			if (!hDlgIcon)
				hDlgIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_BITMAP));

			if (hDlgIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgLargeIconProp, hDlgIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hDlgIcon));
				else
				{
					::DestroyIcon(hDlgIcon);
					hDlgIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(lParam));
			if (pVTI && (sizeof(VIEWTOOLINFO) == pVTI ->cbSize))
			{
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
				HWND hPicView(::GetDlgItem(hDlg, IDC_PICVIEW));
				HWND hButton(::GetDlgItem(hDlg, IDOK));

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hPicView, VSCLASS_EXPLORER, nullptr);
					::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
				}

				DWORD dwDlgBaseWidthUnit(LOWORD(::GetDialogBaseUnits()));
				DWORD dwDlgBaseHeightUnit(HIWORD(::GetDialogBaseUnits()));

				CResString Caption(IDS_BITMAP_DLG_CAPTION);
				::SetWindowTextW(hDlg, Caption.m_bszResString.m_str);

				//	Adjust edit control size/position
				RECT OkButtonRect;
				::GetWindowRect(hButton, &OkButtonRect);
				::MapWindowPoints(HWND_DESKTOP, hDlg, reinterpret_cast<LPPOINT>(&OkButtonRect), sizeof(RECT) / sizeof(POINT));

				RECT DlgClientRect;
				::GetClientRect(hDlg, &DlgClientRect);
				DlgClientRect.right--;
				DlgClientRect.bottom--;

				::MoveWindow(hPicView, 1, 1, DlgClientRect.right - 1, OkButtonRect.top - dwDlgBaseHeightUnit, FALSE);

				if (pVTI ->ResItem.Desc.dwResItemDataSize && pVTI ->ResItem.Desc.lpResItemDataPtr)
				{
					bool fbBitmapIsCreated(false);
					//	Create bitmap

					if (!fbBitmapIsCreated)
					{
						PBITMAPINFO pBMI(reinterpret_cast<PBITMAPINFO>(pVTI ->ResItem.Desc.lpResItemDataPtr));
						DWORD dwBMPSize(*reinterpret_cast<DWORD*>(pBMI));
						DWORD dwCompression;
						DWORD dwnClrUsed;
						PBITMAPCOREHEADER pHeader(nullptr);
						PBITMAPINFOHEADER pBMPIHeader(nullptr);

						PBYTE pBits(nullptr);			
						PRGBTRIPLE pRGBTRIPLE(nullptr);
						LPRGBQUAD pRGBQUAD(nullptr);

						switch(dwBMPSize)
						{
						case sizeof(BITMAPCOREHEADER):
							pHeader = reinterpret_cast<PBITMAPCOREHEADER>(pBMI);
							if ( pHeader->bcBitCount <= 8 )
							{
								pRGBTRIPLE = reinterpret_cast<PRGBTRIPLE>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize);
								pBits = reinterpret_cast<PBYTE>(&pRGBTRIPLE[1 << pHeader->bcBitCount]);
							}
							else
								pBits = reinterpret_cast<PBYTE>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize);
							break;

						case sizeof(BITMAPINFOHEADER):
						case sizeof(BITMAPV4HEADER):
						case sizeof(BITMAPV5HEADER):
							pBMPIHeader = &pBMI->bmiHeader;

							dwCompression = pBMPIHeader ->biCompression;
							dwnClrUsed = pBMPIHeader ->biClrUsed;

							if (pBMPIHeader ->biBitCount <= 8)
								if (!pBMPIHeader ->biClrUsed)
									dwnClrUsed = 1 << pBMPIHeader ->biBitCount;

							if (dwnClrUsed)	// has a color table
							{	
								if (BI_BITFIELDS == dwCompression)
									pRGBQUAD = reinterpret_cast<LPRGBQUAD>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize + 3 * sizeof(DWORD));
								else
									pRGBQUAD = reinterpret_cast<LPRGBQUAD>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize);

								pBits = reinterpret_cast<PBYTE>(&pRGBQUAD[dwnClrUsed]);
							}
							else
							{
								if (BI_BITFIELDS == dwCompression)
									pBits = reinterpret_cast<PBYTE>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize + 3 * sizeof(DWORD));
								else
									pBits = reinterpret_cast<PBYTE>(reinterpret_cast<UINT_PTR>(pBMI) + dwBMPSize);
							}
							break;
						}

						if (pBits)
						{
							HDC hDC(::GetDC(hDlg));
							if (hDC)
							{
								HBITMAP hBMP(::CreateDIBitmap(hDC, &pBMI ->bmiHeader, CBM_INIT, pBits, pBMI, DIB_RGB_COLORS));
								if (hBMP)
								{
									if (::SetPropW(hDlg, lpwszViewSubjResDlgPropHBmp, hBMP))
									{
										::SendMessageW(hPicView, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBMP));
										fbBitmapIsCreated = true;
									}
									else
										::DeleteObject(hBMP);
								}
								::ReleaseDC(hDlg, hDC);
							}
						}
					}

					if (!fbBitmapIsCreated)
					{
						CComPtr<IWICImagingFactory> pIWICFactory;
						pIWICFactory.CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
						if (pIWICFactory.p)
						{
							CComPtr<IWICStream> pIWICStream;
							if (SUCCEEDED(pIWICFactory ->CreateStream(&pIWICStream.p)) && SUCCEEDED(pIWICStream ->InitializeFromMemory(pVTI ->ResItem.Desc.lpResItemDataPtr, pVTI ->ResItem.Desc.dwResItemDataSize)))
							{
								LARGE_INTEGER li = { 0 };
								ULARGE_INTEGER uli = { 0 };

								CComPtr<IWICBitmapDecoder> pIWICBmpDecoder;
								if (SUCCEEDED(pIWICStream ->Seek(li, STREAM_SEEK_SET, &uli)) && SUCCEEDED(pIWICFactory ->CreateDecoderFromStream(pIWICStream.p, nullptr, WICDecodeMetadataCacheOnLoad, &pIWICBmpDecoder.p)))
								{
									CComPtr<IWICBitmapFrameDecode> pSource;
									if (SUCCEEDED(pIWICBmpDecoder ->GetFrame(0, &pSource.p)))
									{
										CComPtr<IWICFormatConverter> pConverter;
										if (SUCCEEDED(pIWICFactory ->CreateFormatConverter(&pConverter.p)))
										{
											HRESULT hR;
											if (SUCCEEDED(hR = pConverter ->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut)))
											{
												UINT cx(0), cy(0);
												if (SUCCEEDED(hR = pConverter ->GetSize(&cx, &cy)))
												{
													const SIZE sizBmp = { static_cast<LONG>(cx), -static_cast<LONG>(cy) };
													BYTE *pbBuffer;
													HBITMAP hBMP(nullptr);
													if (SUCCEEDED(hR = CCoPE32Viewer::Create32BitHBITMAP(nullptr, &sizBmp, reinterpret_cast<void **>(&pbBuffer), &hBMP)) && hBMP)
													{
														const UINT cbStride = cx * sizeof(ARGB);
														const UINT cbBuffer = cy * cbStride;

														if (SUCCEEDED(hR = pConverter ->CopyPixels(nullptr, cbStride, cbBuffer, pbBuffer)))
														{
															if (::SetPropW(hDlg, lpwszViewSubjResDlgPropHBmp, hBMP))
															{
																::SendMessageW(hPicView, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBMP));
																fbBitmapIsCreated = true;
															}
															else
																if (hBMP)
																	::DeleteObject(hBMP);
														}
														else
															if (hBMP)
																::DeleteObject(hBMP);
													}
												}
											}
										}
									}
								}
							}
							pIWICFactory.Release();
						}
					}
				}
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
                ::EndDialog(hDlg, 1);
                return TRUE; 
        } 
		break;
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case SC_CLOSE:
				::EndDialog(hDlg, 1);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			HICON hIcon(reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgSmallIconProp)));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon = reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgLargeIconProp));
			if (hIcon)
				::DestroyIcon(hIcon);

			HBITMAP hBmp(reinterpret_cast<HBITMAP>(::RemovePropW(hDlg, lpwszViewSubjResDlgPropHBmp)));
			if (hBmp)
				::DeleteObject(hBmp);
		}
		break;
	}
	return 0;
}

void ShowNibble(__in BYTE aByte, __out __notnull __wchar_t* pwszWChar)
{
	BYTE aNibble(aByte & 0xF);
	switch(aNibble)
	{
	case 0:
		*pwszWChar = L'0';
		break;
	case 1:
		*pwszWChar = L'1';
		break;
	case 2:
		*pwszWChar = L'2';
		break;
	case 3:
		*pwszWChar = L'3';
		break;
	case 4:
		*pwszWChar = L'4';
		break;
	case 5:
		*pwszWChar = L'5';
		break;
	case 6:
		*pwszWChar = L'6';
		break;
	case 7:
		*pwszWChar = L'7';
		break;
	case 8:
		*pwszWChar = L'8';
		break;
	case 9:
		*pwszWChar = L'9';
		break;
	case 0x0A:
		*pwszWChar = L'A';
		break;
	case 0x0B:
		*pwszWChar = L'B';
		break;
	case 0x0C:
		*pwszWChar = L'C';
		break;
	case 0x0D:
		*pwszWChar = L'D';
		break;
	case 0x0E:
		*pwszWChar = L'E';
		break;
	case 0x0F:
		*pwszWChar = L'F';
		break;
	}
}

void ShowByte(__in BYTE aByte, __out __notnull __wchar_t* pwszWChar)
{
	ShowNibble(aByte >> 4, pwszWChar);
	ShowNibble(aByte, &pwszWChar[1]);
}

void ShowDWord(__in DWORD dwValue, __out __notnull __wchar_t* pwszWChar)
{
	union
	{
		BYTE dwByte[4];
		DWORD dwVal;
	}
	DWord_Value;
	DWord_Value.dwVal = dwValue;

	for (int icnIndex(4); icnIndex > 0; --icnIndex)
		ShowByte(DWord_Value.dwByte[icnIndex - 1], &pwszWChar[4 - icnIndex]);
}

__wchar_t MapChar(__in char aChar)
{
	char aChars[2] = { aChar, 0 };
	__wchar_t wChars[2] = { 0 };
	if (aChar >= 32)
		wChars[0] = static_cast<__wchar_t>(aChar);
	else
		wChars[0] = L' ';
	return (wChars[0]);
}

void WriteMemoryChunk(__in LPCVOID lpMemory, __out __notnull PMEM_CHUNK_VIEW pMCV, __out __maybenull LPVOID* lpNextParagraf)
{
	BYTE* lpPara(reinterpret_cast<BYTE*>(const_cast<LPVOID>(lpMemory)));
	for (int icnIndex(0); icnIndex < MEM_CHUNK_SIZE; ++icnIndex)
		ShowByte(lpPara[icnIndex], pMCV ->Bytes[icnIndex].wszByteView);

	char* lpParaChar(reinterpret_cast<char*>(lpPara));

	for (int icnIndex(0); icnIndex < MEM_CHUNK_SIZE; ++icnIndex)
		pMCV ->Symbols[icnIndex] = MapChar(lpParaChar[icnIndex]);

	if (lpNextParagraf != nullptr)
		*lpNextParagraf = reinterpret_cast<LPVOID>((UINT_PTR)(lpPara) + MEM_CHUNK_SIZE);
}

void AddStringToHexView(__in HWND hReport, __in __notnull PMEM_CHUNK_VIEW pMCV)
{
	int nCount;
	if (::IsWindowUnicode(hReport))
		nCount = ::SendMessageW(hReport, LVM_GETITEMCOUNT, 0, 0);
	else
		nCount = ::SendMessageA(hReport, LVM_GETITEMCOUNT, 0, 0);

	LVITEMW lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = nCount;
	lvi.pszText = pMCV ->Offset;
	lvi.cchTextMax = ARRAYSIZE(pMCV ->Offset);

	int nStringNo = ListView_InsertItem(hReport, &lvi);

	//	Fill other columns
	for (int iIndex(1); iIndex <= MEM_CHUNK_SIZE; ++iIndex)
		ListView_SetItemText(hReport, nStringNo, iIndex, pMCV ->Bytes[iIndex - 1].wszByteView);

	ListView_SetItemText(hReport, nStringNo, HEX_VIEW_COLUMNS - 1, pMCV ->Symbols);
}

void ShowMemory(__in __notnull HWND hReport, __in __notnull LPCVOID lpMemory, __in_bcount(lpMemory) DWORD dwMemorySize)
{
	DWORD dwSize(dwMemorySize);
	
	LPVOID lpBase(const_cast<LPVOID>(lpMemory));
	LPVOID lpBaseDisplay(nullptr);
	LPVOID lpNext(nullptr);
	DWORD dwBlockNo(0);

	DWORD dwFirstChunkSize(MEM_CHUNK_SIZE - (reinterpret_cast<UINT_PTR>(lpBase) & (MEM_CHUNK_SIZE - 1)));	
	DWORD dwFirstChunkGapSize((DWORD)(lpBase) & (MEM_CHUNK_SIZE - 1));	

	if (dwFirstChunkSize)
	{
		MEM_CHUNK_VIEW mcv = { 0 };

		DWORD dwFirstChunk(reinterpret_cast<UINT_PTR>(lpBase) & ~(MEM_CHUNK_SIZE - 1));
		ShowDWord(reinterpret_cast<DWORD>(lpBaseDisplay), mcv.Offset);

		UINT cnIndex(0);
		for (; cnIndex < dwFirstChunkGapSize; ++cnIndex)
			mcv.Bytes[cnIndex].wszByteView[0] = L' ';

		while (cnIndex < MEM_CHUNK_SIZE)
		{
			ShowByte((reinterpret_cast<BYTE*>(lpBase))[cnIndex], mcv.Bytes[cnIndex].wszByteView);
			++cnIndex;
		}

		cnIndex = 0;
		for (; cnIndex < dwFirstChunkGapSize; ++cnIndex)
			mcv.Symbols[cnIndex] = L' ';

		__wchar_t wszSymbol[2] = { 0 };
		while (cnIndex < MEM_CHUNK_SIZE)
		{
			mcv.Symbols[cnIndex] = MapChar((reinterpret_cast<char*>(lpBase))[cnIndex]);
			++cnIndex;
		}

		AddStringToHexView(hReport, &mcv);

		lpBase = reinterpret_cast<LPVOID>(dwFirstChunk + MEM_CHUNK_SIZE);
		lpBaseDisplay = reinterpret_cast<LPVOID>(reinterpret_cast<UINT_PTR>(lpBaseDisplay) + MEM_CHUNK_SIZE);
		dwSize -= dwFirstChunkSize;
	}

	while (dwSize >= MEM_CHUNK_SIZE)
	{
		MEM_CHUNK_VIEW mcv = { 0 };
		ShowDWord(reinterpret_cast<DWORD>(lpBaseDisplay), mcv.Offset);

		WriteMemoryChunk(lpBase, &mcv, &lpNext);
		AddStringToHexView(hReport, &mcv);

		lpBase = lpNext;
		lpNext = nullptr;
		lpBaseDisplay = reinterpret_cast<LPVOID>(reinterpret_cast<UINT_PTR>(lpBaseDisplay) + MEM_CHUNK_SIZE);
		dwSize -= MEM_CHUNK_SIZE;
	}

	if (dwSize)
	{
		MEM_CHUNK_VIEW mcv = { 0 };
		ShowDWord(reinterpret_cast<DWORD>(lpBaseDisplay), mcv.Offset);

		UINT cnIndex(0);
		for (; cnIndex < dwSize; ++cnIndex)
			ShowByte((reinterpret_cast<BYTE*>(lpBase))[cnIndex], mcv.Bytes[cnIndex].wszByteView);

		while (cnIndex < MEM_CHUNK_SIZE)
		{
			mcv.Bytes[cnIndex].wszByteView[0] = L' ';
			++cnIndex;
		}

		cnIndex = 0;
		for (; cnIndex < dwSize; ++cnIndex)
			mcv.Symbols[cnIndex] = MapChar((reinterpret_cast<char*>(lpBase))[cnIndex]);

		while (cnIndex < MEM_CHUNK_SIZE)
		{
			mcv.Symbols[cnIndex] = L' ';
			++cnIndex;
		}

		AddStringToHexView(hReport, &mcv);
	}
}

INT_PTR CALLBACK CPE32PropsView::ChildViewAsBlobDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hDlgSmIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
			if (!hDlgSmIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), LIM_SMALL, &hDlgSmIcon);
			if (!hDlgSmIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hDlgSmIcon);
			if (!hDlgSmIcon)
				hDlgSmIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX));

			if (hDlgSmIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgSmallIconProp, hDlgSmIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
				else
				{
					::DestroyIcon(hDlgSmIcon);
					hDlgSmIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			HICON hDlgIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)));
			if (!hDlgIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), LIM_LARGE, &hDlgIcon);
			if (!hDlgIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX), ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), &hDlgIcon);
			if (!hDlgIcon)
				hDlgIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_HEX));

			if (hDlgIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgLargeIconProp, hDlgIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hDlgIcon));
				else
				{
					::DestroyIcon(hDlgIcon);
					hDlgIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(lParam));
			if (pVTI && (sizeof(VIEWTOOLINFO) == pVTI ->cbSize))
			{
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
				HWND hReport(::GetDlgItem(hDlg, IDC_HEXVIEW));
				HWND hButton(::GetDlgItem(hDlg, IDOK));

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hReport, VSCLASS_EXPLORER, nullptr);
					::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
				}

				DWORD dwDlgBaseWidthUnit(LOWORD(::GetDialogBaseUnits()));
				DWORD dwDlgBaseHeightUnit(HIWORD(::GetDialogBaseUnits()));

				CResString Caption(IDS_UNICODE_DLG_CAPTION);
				::SetWindowTextW(hDlg, Caption.m_bszResString.m_str);

				//	Adjust edit control size/position
				RECT OkButtonRect;
				::GetWindowRect(hButton, &OkButtonRect);
				::MapWindowPoints(HWND_DESKTOP, hDlg, reinterpret_cast<LPPOINT>(&OkButtonRect), sizeof(RECT) / sizeof(POINT));

				RECT DlgClientRect;
				::GetClientRect(hDlg, &DlgClientRect);
				DlgClientRect.right--;
				DlgClientRect.bottom--;

				::MoveWindow(hReport, 1, 1, DlgClientRect.right - 1, OkButtonRect.top - dwDlgBaseHeightUnit, FALSE);

				LVCOLUMNW lvc = { 0 };
				lvc.mask = LVCF_FMT | LVCF_WIDTH;
				lvc.fmt = LVCFMT_LEFT;

				int nOffsetWidth(ListView_GetStringWidth(hReport, L"XXXXXXXX"));
				lvc.cx = nOffsetWidth;
				ListView_InsertColumn(hReport, 0, &lvc);

				int nSymbolsWidth(ListView_GetStringWidth(hReport, L"WWWWWWWWW "));

				RECT rcReport;
				::GetClientRect(hReport, &rcReport);

				int nByteViewWidth((rcReport.right - rcReport.left - nOffsetWidth - nSymbolsWidth - ListView_GetStringWidth(hReport, L"W") - MEM_CHUNK_SIZE) / MEM_CHUNK_SIZE);
				for (int nIndex(1); nIndex <= MEM_CHUNK_SIZE; ++nIndex)
				{
					lvc.fmt = LVCFMT_CENTER;
					lvc.cx = nByteViewWidth;
					ListView_InsertColumn(hReport, nIndex, &lvc);
				}

				lvc.fmt = LVCFMT_LEFT;
				lvc.cx = nSymbolsWidth;
				ListView_InsertColumn(hReport, HEX_VIEW_COLUMNS - 1, &lvc);

				ShowMemory(hReport, pVTI ->ResItem.Desc.lpResItemDataPtr, pVTI ->ResItem.Desc.dwResItemDataSize);
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
                ::EndDialog(hDlg, 1);
                return TRUE; 
        } 
		break;
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case SC_CLOSE:
				::EndDialog(hDlg, 1);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			HICON hIcon(reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgSmallIconProp)));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon = reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgLargeIconProp));
			if (hIcon)
				::DestroyIcon(hIcon);
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK CPE32PropsView::ChildViewAsUnicodeDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hDlgSmIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
			if (!hDlgSmIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), LIM_SMALL, &hDlgSmIcon);
			if (!hDlgSmIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hDlgSmIcon);
			if (!hDlgSmIcon)
				hDlgSmIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE));

			if (hDlgSmIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgSmallIconProp, hDlgSmIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
				else
				{
					::DestroyIcon(hDlgSmIcon);
					hDlgSmIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			HICON hDlgIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)));
			if (!hDlgIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), LIM_LARGE, &hDlgIcon);
			if (!hDlgIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE), ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), &hDlgIcon);
			if (!hDlgIcon)
				hDlgIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_UNICODE));

			if (hDlgIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgLargeIconProp, hDlgIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hDlgIcon));
				else
				{
					::DestroyIcon(hDlgIcon);
					hDlgIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(lParam));
			if (pVTI && (sizeof(VIEWTOOLINFO) == pVTI ->cbSize))
			{
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
				HWND hEdit(::GetDlgItem(hDlg, IDC_EDT_UNICODEVIEW));
				HWND hButton(::GetDlgItem(hDlg, IDOK));

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hEdit, VSCLASS_EXPLORER, nullptr);
					::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
				}

				CResString Caption(IDS_UNICODE_DLG_CAPTION);
				::SetWindowTextW(hDlg, Caption.m_bszResString.m_str);

				DWORD dwDlgBaseWidthUnit(LOWORD(::GetDialogBaseUnits()));
				DWORD dwDlgBaseHeightUnit(HIWORD(::GetDialogBaseUnits()));

				//	Adjust edit control size/position
				RECT OkButtonRect;
				::GetWindowRect(hButton, &OkButtonRect);
				::MapWindowPoints(HWND_DESKTOP, hDlg, reinterpret_cast<LPPOINT>(&OkButtonRect), sizeof(RECT) / sizeof(POINT));

				RECT DlgClientRect;
				::GetClientRect(hDlg, &DlgClientRect);
				DlgClientRect.right--;
				DlgClientRect.bottom--;

				::MoveWindow(hEdit, 1, 1, DlgClientRect.right - 1, OkButtonRect.top - dwDlgBaseHeightUnit, FALSE);				

				if (pVTI ->ResItem.Desc.dwResItemDataSize)
				{
					auto Deleter1 = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
					std::unique_ptr<__wchar_t, decltype(Deleter1)> pszTmp(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(((pVTI ->ResItem.Desc.dwResItemDataSize + 1) & WIDECHAR_MASK) + sizeof(__wchar_t))), Deleter1);

					if (pszTmp)
					{
						::ZeroMemory(pszTmp.get(), ((pVTI ->ResItem.Desc.dwResItemDataSize + 1) & WIDECHAR_MASK) + sizeof(__wchar_t));
						::CopyMemory(pszTmp.get(), pVTI ->ResItem.Desc.lpResItemDataPtr, pVTI ->ResItem.Desc.dwResItemDataSize);

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
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
                ::EndDialog(hDlg, 1);
                return TRUE; 
        } 
		break;
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case SC_CLOSE:
				::EndDialog(hDlg, 1);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			HICON hIcon(reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgSmallIconProp)));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon = reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgLargeIconProp));
			if (hIcon)
				::DestroyIcon(hIcon);
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK CPE32PropsView::ChildViewAsAnsiDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hDlgSmIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
			if (!hDlgSmIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), LIM_SMALL, &hDlgSmIcon);
			if (!hDlgSmIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hDlgSmIcon);
			if (!hDlgSmIcon)
				hDlgSmIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI));

			if (hDlgSmIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgSmallIconProp, hDlgSmIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
				else
				{
					::DestroyIcon(hDlgSmIcon);
					hDlgSmIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			HICON hDlgIcon(static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)));
			if (!hDlgIcon)
				::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), LIM_LARGE, &hDlgIcon);
			if (!hDlgIcon)
				::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI), ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), &hDlgIcon);
			if (!hDlgIcon)
				hDlgIcon = ::LoadIcon(hDLLInstance, MAKEINTRESOURCEW(IDI_ANSI));

			if (hDlgIcon)
			{
				if (::SetPropW(hDlg, lpwszViewResToolDlgLargeIconProp, hDlgIcon))
					::SendMessageW(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hDlgIcon));
				else
				{
					::DestroyIcon(hDlgIcon);
					hDlgIcon = nullptr;
				}
			}
			else
			{
				if (hDlgSmIcon = static_cast<HICON>(::LoadImageW(nullptr, MAKEINTRESOURCEW(IDI_INFORMATION), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR)))
					::SendMessageW(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hDlgSmIcon));
			}

			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(lParam));
			if (pVTI && (sizeof(VIEWTOOLINFO) == pVTI ->cbSize))
			{
				CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
				HWND hEdit(::GetDlgItem(hDlg, IDC_EDT_ANSI_VIEW));
				HWND hButton(::GetDlgItem(hDlg, IDOK));

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hEdit, VSCLASS_EXPLORER, nullptr);
					::SetWindowTheme(hButton, VSOKBUTTON, nullptr);
				}

				CResString Caption(IDS_ANSI_DLG_CAPTION);
				::SetWindowTextW(hDlg, Caption.m_bszResString.m_str);

				DWORD dwDlgBaseWidthUnit(LOWORD(::GetDialogBaseUnits()));
				DWORD dwDlgBaseHeightUnit(HIWORD(::GetDialogBaseUnits()));

				//	Adjust edit control size/position
				RECT OkButtonRect;
				::GetWindowRect(hButton, &OkButtonRect);
				::MapWindowPoints(HWND_DESKTOP, hDlg, reinterpret_cast<LPPOINT>(&OkButtonRect), sizeof(RECT) / sizeof(POINT));

				RECT DlgClientRect;
				::GetClientRect(hDlg, &DlgClientRect);
				DlgClientRect.right--;
				DlgClientRect.bottom--;

				::MoveWindow(hEdit, 1, 1, DlgClientRect.right - 1, OkButtonRect.top - dwDlgBaseHeightUnit, FALSE);				

				int cniBufferLen;
				DWORD dwFlags(MB_PRECOMPOSED);
				switch(pVTI ->ResItem.Desc.dwCodePage)
				{
				case 42:
				case 50220:
				case 50221:
				case 50222:
				case 50225:
				case 50227:
				case 50229:
				case 54936:
				case 57002:
				case 57003:
				case 57004:
				case 57005:
				case 57006:
				case 57007:
				case 57008:
				case 57009:
				case 57010:
				case 57011:
				case CP_UTF7:
				case CP_UTF8:
					dwFlags = 0;
					break;
				default:
					dwFlags = MB_PRECOMPOSED;
					break;
				}

				if (0 != (cniBufferLen = ::MultiByteToWideChar(pVTI ->ResItem.Desc.dwCodePage, dwFlags, reinterpret_cast<char*>(pVTI ->ResItem.Desc.lpResItemDataPtr), static_cast<int>(pVTI ->ResItem.Desc.dwResItemDataSize), nullptr, 0)))
				{
					auto Deleter1 = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
					std::unique_ptr<__wchar_t, decltype(Deleter1)> pszTmp(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) * (cniBufferLen + 1))), Deleter1);

					if (pszTmp)
					{
						::ZeroMemory(pszTmp.get(), (cniBufferLen + 1) * sizeof(__wchar_t));
						if (0 != ::MultiByteToWideChar(pVTI ->ResItem.Desc.dwCodePage, dwFlags, reinterpret_cast<char*>(pVTI ->ResItem.Desc.lpResItemDataPtr), static_cast<int>(pVTI ->ResItem.Desc.dwResItemDataSize), pszTmp.get(), cniBufferLen))
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
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
                ::EndDialog(hDlg, 1);
                return TRUE; 
        } 
		break;
	case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
			case SC_CLOSE:
				::EndDialog(hDlg, 1);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			HICON hIcon(reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgSmallIconProp)));
			if (hIcon)
				::DestroyIcon(hIcon);
			hIcon = reinterpret_cast<HICON>(::RemovePropW(hDlg, lpwszViewResToolDlgLargeIconProp));
			if (hIcon)
				::DestroyIcon(hIcon);
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK CPE32PropsView::ChildResourcesDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTree(::GetDlgItem(hDlg, IDC_RESOURCES_TREE));
			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			}

			if (pView)
			{
				::SetPropW(hDlg, lpwszResourcesDlgProp, pView);

				//	Prepare mf_UnkResItem
				int cnCount(0);
				if (pView ->mf_UnkResItem.mf_HDSA && (cnCount = DSA_GetItemCount(pView ->mf_UnkResItem.mf_HDSA)))
					::DSA_DeleteAllItems(pView ->mf_UnkResItem.mf_HDSA);

				RESOURCES_ENUM_CONTEXT rec;
				rec.dwStructSize = sizeof(RESOURCES_ENUM_CONTEXT);
				rec.pMainDlg = pView;
				rec.hDlg = hDlg;
				rec.hTree = hTree;

				pView ->mf_pPE32 ->EnumPEImgResourceTypes(dynamic_cast<IPE32ResourcesCallback*>(pView), &rec);
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left/* + ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.top/* + ::GetSystemMetrics(SM_CYDLGFRAME)*/, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left/* - 2 * ::GetSystemMetrics(SM_CXDLGFRAME)*/, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top/* - 2 * ::GetSystemMetrics(SM_CYDLGFRAME)*/, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hTree, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);

		}
		break;
	case WM_DESTROY:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::RemovePropW(hDlg, lpwszResourcesDlgProp)));
			if (pView && pView ->mf_UnkResItem.mf_HDSA && DSA_GetItemCount(pView ->mf_UnkResItem.mf_HDSA))
				::DSA_DeleteAllItems(pView ->mf_UnkResItem.mf_HDSA);

			void* pTmp(::RemovePropW(hDlg, lpwszViewSubjResDlgProp));
			if (pTmp)
				::CoTaskMemFree(pTmp);
		}
		break;
	case WM_COMMAND:
		{
			PVIEWTOOLINFO pVTI(reinterpret_cast<PVIEWTOOLINFO>(::GetPropW(hDlg, lpwszViewSubjResDlgProp)));
			VIEWTOOLINFO pVTITmp = { 0 };

			if (pVTI /*&& (sizeof(VIEWTOOLINFO) == pVTI ->cbSize)*/)
			{
				::CopyMemory(&pVTITmp, pVTI, sizeof(VIEWTOOLINFO));
				if (0 == HIWORD(wParam))
				{
					switch(LOWORD(wParam))
					{
					case IDC_VIEWASANSITEXT:
						::DialogBoxIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_ANSIVIEW)), pVTI ->pMainDlg ->m_hWnd, ChildViewAsAnsiDlgInitProc, reinterpret_cast<LPARAM>(&pVTITmp));
						break;
					case IDC_VIEWASUNICODETEXT:
						::DialogBoxIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_UNICODEVIEW)), pVTI ->pMainDlg ->m_hWnd, ChildViewAsUnicodeDlgInitProc, reinterpret_cast<LPARAM>(&pVTITmp));
						break;
					case IDC_VIEWASBLOB:
						::DialogBoxIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_HEXVIEW)), pVTI ->pMainDlg ->m_hWnd, ChildViewAsBlobDlgInitProc, reinterpret_cast<LPARAM>(&pVTITmp));
						break;
					case IDC_SHOWASICON:
						::DialogBoxIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_ICONVIEW)), pVTI ->pMainDlg ->m_hWnd, ChildViewAsIconDlgInitProc, reinterpret_cast<LPARAM>(&pVTITmp));
						break;
					case IDC_SHOWASBITMAP:
						::DialogBoxIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_BITMAPVIEW)), pVTI ->pMainDlg ->m_hWnd, ChildViewAsBitmapDlgInitProc, reinterpret_cast<LPARAM>(&pVTITmp));
						break;
					}
				}
			}
			else
			{
				::MessageBoxW(hDlg, L"View tool info is NOT available", L"Menu command", MB_OK);
			}
		}
		break;
	case WM_CONTEXTMENU:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszResourcesDlgProp)));
			if (pView)
			{
				HWND hTree(::GetDlgItem(hDlg, IDC_RESOURCES_TREE));
				if (hTree == reinterpret_cast<HWND>(wParam))
				{
					TVHITTESTINFO tvhti = { 0 };
					tvhti.pt.x = GET_X_LPARAM(lParam);
					tvhti.pt.y = GET_Y_LPARAM(lParam);

					::MapWindowPoints(HWND_DESKTOP, hTree, &tvhti.pt, 1);

					HTREEITEM hti(TreeView_HitTest(hTree, &tvhti));
					//ScreenToClient
					int nResItemIndex(-1);
					if (hti)
					{
						__wchar_t lpwszInfoStr[TMP_BUFFER_LENGTH] = { 0 };
						__wchar_t szTVIText[TMP_BUFFER_LENGTH] = { 0 };

						//You can create a header control by using the CreateWindowEx function, specifying the WC_HEADER 

						if (GetCommCtl32DllVersion() >= PACKVERSION(4,71))
						{
							TVITEMEXW tvi = { 0 };
							tvi.mask = TVIF_TEXT | TVIF_PARAM;
							tvi.hItem = hti;
							tvi.cchTextMax = TMP_BUFFER_LENGTH;
							tvi.pszText = szTVIText;
							if (TreeView_GetItem(hTree, &tvi))
							{
								//::StringCchPrintfW(lpwszInfoStr, TMP_BUFFER_LENGTH, L"Tree view item is found (Ex)\nIndex: %d\nText: %s", static_cast<int>(tvi.lParam) - 1, tvi.pszText);
								//::MessageBoxW(hDlg, lpwszInfoStr, L"TreeView_HitTest", MB_OK);
								nResItemIndex = static_cast<int>(tvi.lParam) - 1;
							}
						}
						else
						{
							TVITEMW tvi = { 0 }; 
							tvi.mask = TVIF_TEXT | TVIF_PARAM;
							tvi.hItem = hti;
							tvi.cchTextMax = TMP_BUFFER_LENGTH;
							tvi.pszText = szTVIText;
							if (TreeView_GetItem(hTree, &tvi))
							{
								//::StringCchPrintfW(lpwszInfoStr, TMP_BUFFER_LENGTH, L"Tree view item is found\nIndex: %d\nText: %s", static_cast<int>(tvi.lParam) - 1, tvi.pszText);
								//::MessageBoxW(hDlg, lpwszInfoStr, L"TreeView_HitTest", MB_OK);
								nResItemIndex = static_cast<int>(tvi.lParam) - 1;
							}
						}
						
						int cnItemCount(0);
						UNK_RES_ITEM uri = { 0 };
						if (pView ->mf_UnkResItem.mf_HDSA && (cnItemCount = DSA_GetItemCount(pView ->mf_UnkResItem.mf_HDSA)) && (nResItemIndex >= 0 && nResItemIndex < cnItemCount) && ::DSA_GetItem(pView ->mf_UnkResItem.mf_HDSA, nResItemIndex, &uri))
						{
							VIEWTOOLINFO vti = { 0 };
							vti.cbSize = sizeof(VIEWTOOLINFO);
							vti.nItemIndex = nResItemIndex;
							vti.pMainDlg = pView;
							::CopyMemory(&vti.ResItem, &uri, sizeof(UNK_RES_ITEM));

							PVIEWTOOLINFO pvti;
							if (pvti = reinterpret_cast<PVIEWTOOLINFO>(::GetPropW(hDlg, lpwszViewSubjResDlgProp)))
								::CopyMemory(pvti, &vti, sizeof(VIEWTOOLINFO));
							else
							{
								auto Deleter_1 = [](PVIEWTOOLINFO _pvti) { if (_pvti) ::CoTaskMemFree(_pvti); };
								std::unique_ptr<VIEWTOOLINFO, decltype(Deleter_1)> pVTITmp(reinterpret_cast<PVIEWTOOLINFO>(::CoTaskMemAlloc(sizeof(VIEWTOOLINFO))), Deleter_1);
								::CopyMemory(pVTITmp.get(), &vti, sizeof(VIEWTOOLINFO));
								if (::SetPropW(hDlg, lpwszViewSubjResDlgProp, pVTITmp.get()))
									pVTITmp.release();
							}

							//	Load appropriate context menu
							UINT nMenuResID(IDR_CTX_BLOB_MENU);
							switch(uri.dwResType)
							{
							case 2:	//Bitmap
								nMenuResID = IDR_CTX_BITMAP_MENU;
								break;
							case 1:	//Cursor
							case 3:	//Icon
								nMenuResID = IDR_CTX_ICON_MENU;
								break;
							}

							HMENU hMenu(::LoadMenuW(hDLLInstance, MAKEINTRESOURCEW(nMenuResID)));
							if (hMenu)
							{
								HMENU hSubMenu(::CreatePopupMenu());
								if (::IsThemeActive())
									::SetWindowTheme(reinterpret_cast<HWND>(hMenu), VSCLASS_MENU, nullptr);
								HMENU hPopupMenu(::GetSubMenu(hMenu, 0));

								//VSCLASS_MENU
								int cnValidBmp(0);
								HBITMAP hBmps[MENU_BITMAP_MAX] = { 0 };

								if (hPopupMenu)
								{
									::SetWindowTheme(reinterpret_cast<HWND>(hPopupMenu), VSCLASS_MENU, nullptr);
									UINT uiTPMFlags(TPM_CENTERALIGN | TPM_VCENTERALIGN /*| TPM_RETURNCMD*/ | (::GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTBUTTON : TPM_LEFTBUTTON));

									BOOL fbMenuAnimation(FALSE);
									if (::SystemParametersInfoW(SPI_GETMENUANIMATION, sizeof(BOOL), &fbMenuAnimation, 0) && fbMenuAnimation)
									{
										BOOL fbMenuFade(FALSE);
										if (::SystemParametersInfoW(SPI_GETMENUFADE, sizeof(BOOL), &fbMenuFade, 0) && !fbMenuFade)
											uiTPMFlags |= TPM_VERNEGANIMATION | TPM_HORPOSANIMATION;
									}

									int nItemsMax(::GetMenuItemCount(hPopupMenu));
									for (int nIndex(0); nIndex < nItemsMax; ++nIndex)
									{
										MENUITEMINFOW mii = { 0 };
										mii.cbSize = sizeof(MENUITEMINFOW);
										mii.fMask = MIIM_FTYPE | MIIM_ID;

										if (::GetMenuItemInfoW(hPopupMenu, static_cast<UINT>(nIndex), TRUE, &mii))
										{
											if (MF_SEPARATOR == (mii.fType & MF_SEPARATOR))
												continue;

											BOOL fbBmpCreated(FALSE);

											//HICON hIcon;
											//	Try to create bitmap
											UINT uiResID(0);
											switch(mii.wID)
											{
											case IDC_VIEWASANSITEXT:
												uiResID = IDI_ANSI;
												break;
											case IDC_VIEWASUNICODETEXT:
												uiResID = IDI_UNICODE;
												break;
											case IDC_VIEWASBLOB:
												uiResID = IDI_HEX;
												break;
											case IDC_SHOWASICON:
												uiResID = IDI_ICO;
												break;
											case IDC_SHOWASBITMAP:
												uiResID = IDI_BITMAP;
												break;
											}

											int nID(0);
											if (uiResID)
											{
												HICON hIcon(nullptr);

												if ((S_OK != ::LoadIconWithScaleDown(hDLLInstance, MAKEINTRESOURCEW(LOWORD(uiResID)), ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), &hIcon)) && (S_OK != ::LoadIconMetric(hDLLInstance, MAKEINTRESOURCEW(LOWORD(uiResID)), LIM_SMALL, &hIcon)) && !hIcon)
													hIcon = static_cast<HICON>(::LoadImageW(hDLLInstance, MAKEINTRESOURCEW(LOWORD(uiResID)), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
												if (!hIcon)
														hIcon = ::LoadIconW(hDLLInstance, MAKEINTRESOURCEW(uiResID));

												if (hIcon)
												{
													CComPtr<IWICImagingFactory> pFactory;
													pFactory.CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER);
													if (pFactory.p)
														CCoPE32Viewer::AddIconToMenuItem(pFactory.p, hIcon, FALSE, &hBmps[cnValidBmp]);
													if (!hBmps[cnValidBmp])
														CCoPE32Viewer::AddIconToMenuItemNoWIC(hIcon, FALSE, &hBmps[cnValidBmp]);
													if (!hBmps[cnValidBmp])
														CCoPE32Viewer::AddIconToMenuItemGDI(hIcon, FALSE, &hBmps[cnValidBmp]);

													if (hBmps[cnValidBmp])
													{
														++cnValidBmp;
														fbBmpCreated = TRUE;
													}
													::DestroyIcon(hIcon);
													hIcon = nullptr;
												}
											}

											if (fbBmpCreated)
											{
												mii.fMask |= MIIM_BITMAP;
												mii.hbmpItem = hBmps[cnValidBmp - 1];
												if (::SetMenuItemInfoW(hPopupMenu, static_cast<UINT>(nIndex), TRUE, &mii))
													::DrawMenuBar(hDlg);
											}											
										}
									}

									::TrackPopupMenuEx(hPopupMenu, uiTPMFlags, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), hDlg, nullptr);
								}
								::DestroyMenu(hMenu);

								for (int nIndex(0); nIndex < MENU_BITMAP_MAX; ++nIndex)
								{
									if (hBmps[nIndex])
									{
										::DeleteObject(hBmps[nIndex]);
										hBmps[nIndex] = nullptr;
									}
								}
							}
						}						
					}
					else
					{
						::MessageBoxW(hDlg, L"No tree view item", L"TVM_HITTEST", MB_OK);
					}
				}
			}
		}
		break;
	}
	return 0;
}

WORD CPE32PropsView::DecodeResourceID(__in DWORD dwResID)
{
	WORD nID(IDS_RES_TYPE_UNKNOWN);
	switch(dwResID)
	{
	case 1:
		nID = IDS_RES_TYPE_CURSOR;
		break;
	case 2:
		nID = IDS_RES_TYPE_BITMAP;
		break;
	case 3:
		nID = IDS_RES_TYPE_ICON;
		break;
	case 4:
		nID = IDS_RES_TYPE_MENU;
		break;
	case 5:
		nID = IDS_RES_TYPE_DIALOG;
		break;
	case 6:
		nID = IDS_RES_TYPE_STRINGTABLE;
		break;
	case 7:
		nID = IDS_RES_TYPE_FONTDIR;
		break;
	case 8:
		nID = IDS_RES_TYPE_FONT;
		break;
	case 9:
		nID = IDS_RES_TYPE_ACCEL;
		break;
	case 10:
		nID = IDS_RES_TYPE_RCDATA;
		break;
	case 11:
		nID = IDS_RES_TYPE_MSGTABLE;
		break;
	case 12:
		nID = IDS_RES_TYPE_GROUPCURSOR;
		break;
	case 14:
		nID = IDS_RES_TYPE_GROUPICON;
		break;
	case 16:
		nID = IDS_RES_TYPE_VERSION;		//	RT_VERSION
		break;
	case 17:
		nID = IDS_RES_TYPE_DLGINCL;
		break;
	case 19:
		nID = IDS_RES_TYPE_PNP;
		break;
	case 20:
		nID = IDS_RES_TYPE_VXD;
		break;
	case 21:
		nID = IDS_RES_TYPE_ANICURSOR;
		break;
	case 22:
		nID = IDS_RES_TYPE_ANIICON;
		break;
	case 23:
		nID = IDS_RES_TYPE_HTML;
		break;
	case 24:
		nID = IDS_RES_TYPE_MANIFEST;	//RT_MANIFEST
		break;
	}
	return nID;
}

bool CALLBACK CPE32PropsView::EnumBitmapTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;
		__wchar_t szItem[TMP_LARGE_BUFFER_LENGTH] = { 0 };
		PBITMAPINFOHEADER lpBM(reinterpret_cast<PBITMAPINFOHEADER>(pResItemDesc ->lpResItemDataPtr));
		CResString BmpPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_BITMAP_NAME : IDS_RES_TYPE_BITMAP_ID);

		if (rtByString == pResItemDesc ->ResIDType)
			::StringCchPrintfW(szItem, TMP_LARGE_BUFFER_LENGTH, BmpPattern.m_bszResString.m_str, pResItemDesc ->pwszName, pResItemDesc ->dwResItemDataSize, lpBM ->biWidth, lpBM ->biHeight, lpBM ->biBitCount);
		else
			::StringCchPrintfW(szItem, TMP_LARGE_BUFFER_LENGTH, BmpPattern.m_bszResString.m_str, pResItemDesc ->dwID, pResItemDesc ->dwResItemDataSize, lpBM ->biWidth, lpBM ->biHeight, lpBM ->biBitCount);

		int nIndex(DSA_ERR);
		if (pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA)
		{
			UNK_RES_ITEM uri = { 0 };
			uri.cbStructSize = sizeof(UNK_RES_ITEM);
			uri.dwResType = dwResType;
			::CopyMemory(&uri.Desc, pResItemDesc, sizeof(__PEIMGRESOURCEITEM));

			nIndex = ::DSA_InsertItem(pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA, DSA_APPEND, &uri);
		}

		CComBSTR Item = szItem;
		AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Item, nIndex + 1);
		++pRTEFC ->dwIndex;
	}
	return fbResult;
}

bool CALLBACK CPE32PropsView::EnumIconTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;
		__wchar_t szItem[TMP_BUFFER_LENGTH] = { 0 };

		LONG nWidth(0);
		LONG nHeight(0);

		HICON hIcon(nullptr);
		if (hIcon = ::CreateIconFromResourceEx(pResItemDesc ->lpResItemDataPtr, pResItemDesc ->dwResItemDataSize, TRUE, 0x00030000, 0, 0, 0))
		{
			ICONINFOEXW iix = { 0 };
			if (::GetIconInfoExW(hIcon, &iix))
			{
				BITMAP Bmp = { 0 };
				if (iix.hbmColor && sizeof(BITMAP) == ::GetObjectW(iix.hbmColor, sizeof(BITMAP), &Bmp))
				{
					nWidth = Bmp.bmWidth;
					nHeight = Bmp.bmHeight < 0 ? -Bmp.bmHeight : Bmp.bmHeight;
					CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME : IDS_RES_TYPE_ICON_ID);
					::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize, min(nWidth, nHeight), min(nWidth, nHeight));
				}
				else
				{
					CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME_SM : IDS_RES_TYPE_ICON_ID_SM);
					::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize);
				}

				if (iix.hbmColor)
					::DeleteObject(iix.hbmColor);
				if (iix.hbmMask)
					::DeleteObject(iix.hbmColor);
			}
			else
			{
				LPICONIMAGE lpIconDir(reinterpret_cast<LPICONIMAGE>(pResItemDesc ->lpResItemDataPtr));
				if (lpIconDir ->icHeader.biSize == sizeof(BITMAPINFOHEADER))
				{
					nWidth = lpIconDir ->icHeader.biWidth < 0 ? -lpIconDir ->icHeader.biWidth : lpIconDir ->icHeader.biWidth;
					nHeight = lpIconDir ->icHeader.biHeight < 0 ? -lpIconDir ->icHeader.biHeight : lpIconDir ->icHeader.biHeight;

					CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME : IDS_RES_TYPE_ICON_ID);
					::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize, min(nWidth, nHeight), min(nWidth, nHeight));
				}
				else
				{
					CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME_SM : IDS_RES_TYPE_ICON_ID_SM);
					::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize);
				}
			}

			::DestroyIcon(hIcon);
		}
		else
		{
			LPICONIMAGE lpIconDir(reinterpret_cast<LPICONIMAGE>(pResItemDesc ->lpResItemDataPtr));
			if (lpIconDir ->icHeader.biSize == sizeof(BITMAPINFOHEADER))
			{
				nWidth = lpIconDir ->icHeader.biWidth < 0 ? -lpIconDir ->icHeader.biWidth : lpIconDir ->icHeader.biWidth;
				nHeight = lpIconDir ->icHeader.biHeight < 0 ? -lpIconDir ->icHeader.biHeight : lpIconDir ->icHeader.biHeight;

				CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME : IDS_RES_TYPE_ICON_ID);
				::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize, min(nWidth, nHeight), min(nWidth, nHeight));
			}
			else
			{
				CResString IconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_ICON_NAME_SM : IDS_RES_TYPE_ICON_ID_SM);
				::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, IconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize);
			}
		}

		int nIndex(DSA_ERR);
		if (pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA)
		{
			UNK_RES_ITEM uri = { 0 };
			uri.cbStructSize = sizeof(UNK_RES_ITEM);
			uri.dwResType = dwResType;
			::CopyMemory(&uri.Desc, pResItemDesc, sizeof(__PEIMGRESOURCEITEM));

			nIndex = ::DSA_InsertItem(pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA, DSA_APPEND, &uri);
		}

		CComBSTR Item = szItem;
		AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Item, nIndex + 1);
		++pRTEFC ->dwIndex;
	}
	return fbResult;
}

STDMETHODIMP CPE32PropsView::EnumPE32ResourcesTypes(__in unsigned long dwResType, __in BSTR bstrResTypeName, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PRESOURCES_ENUM_CONTEXT pREC(reinterpret_cast<PRESOURCES_ENUM_CONTEXT>(pContext));
	if (pREC && sizeof(RESOURCES_ENUM_CONTEXT) == pREC ->dwStructSize)
	{
		hResult = S_OK;
		CResString ResourceTypeName(DecodeResourceID(dwResType));

		RES_TYPE_ENUM_CONTEXT rtec;
		rtec.dwStructSize = sizeof(RES_TYPE_ENUM_CONTEXT);
		rtec.pMainDlg = pREC ->pMainDlg;
		rtec.hDlg = pREC ->hDlg;
		rtec.hTree = pREC ->hTree;

		switch(dwResType)
		{
		case 1:		//	Cursor
		case 3:		//	Icon
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					rtec.pfnEnumResByType = EnumIconTypeResources;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		case 2:
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					rtec.pfnEnumResByType = EnumBitmapTypeResources;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		case 6:		//	String table
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourceStrings(dynamic_cast<IPE32StringsCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		case 11:
			{	//IDS_RES_TYPE_MSGTABLE
				if (ResourceTypeName.m_bszResString)
				{
					HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
					if (hTreeItem)
					{
						rtec.hti = hTreeItem;
						pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgMessageTableItems(dynamic_cast<IPE32MessagesCallback*>(pREC ->pMainDlg), &rtec);
					}
				}
			}
			break;
		case 12:	//	Group Cursor
		case 14:	//	Group Icon
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					rtec.pfnEnumResByType = EnumGroupIconTypeResources;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		case 16:	//	Version info
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					rtec.pfnEnumResByType = EnumPEImgVersionInfo;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		case 24:	//	Manifest
			if (ResourceTypeName.m_bszResString)
			{
				HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResourceTypeName.m_bszResString));
				if (hTreeItem)
				{
					rtec.hti = hTreeItem;
					rtec.pfnEnumResByType = EnumManifestResourcesByType;
					pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
				}
			}
			break;
		default:
			{				
				__wchar_t szResTypeName[TMP_BUFFER_LENGTH] = { 0 };
				HRESULT hR;
				if (bstrResTypeName)
					hR = ::StringCchPrintfW(szResTypeName, TMP_BUFFER_LENGTH, L"%s (%#08.8x)", bstrResTypeName, dwResType);
				else
					hR = ::StringCchPrintfW(&szResTypeName[0], TMP_BUFFER_LENGTH, ResourceTypeName.m_bszResString, dwResType);

				if (SUCCEEDED(hR))
				{
					CComBSTR ResTypeName = szResTypeName;
					HTREEITEM hTreeItem(AddItemToTreeView(pREC ->hTree, nullptr, TVI_ROOT, ResTypeName));
					if (hTreeItem)
					{
						rtec.hti = hTreeItem;
						rtec.pfnEnumResByType = EnumResourcesByType;
						pREC ->pMainDlg ->mf_pPE32 ->EnumPEImgResourcesByType(dwResType, dynamic_cast<IPE32ResourcesCallback*>(pREC ->pMainDlg), &rtec);
					}
				}
			}
			break;
		}
	}

	if (bstrResTypeName)
		::SysFreeString(bstrResTypeName);

	return hResult;
}

STDMETHODIMP CPE32PropsView::EnumPE32ResourcesByType(__in unsigned long dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PRES_TYPE_ENUM_CONTEXT pRTEC(reinterpret_cast<PRES_TYPE_ENUM_CONTEXT>(pContext));
	if (pRTEC && sizeof(RES_TYPE_ENUM_CONTEXT) == pRTEC ->dwStructSize)
	{	
		RES_TYPE_ENUM_FN_CONTEXT rtefnc;
		rtefnc.dwStructSize = sizeof(RES_TYPE_ENUM_FN_CONTEXT);
		rtefnc.pMainDlg = pRTEC ->pMainDlg;
		rtefnc.hDlg = pRTEC ->hDlg;
		rtefnc.hTree = pRTEC ->hTree;
		rtefnc.hti = pRTEC ->hti;
		rtefnc.dwIndex = 0;

		if (pRTEC ->pfnEnumResByType)
		{
			if ((*pRTEC ->pfnEnumResByType)(rtefnc.dwIndex, dwResType, pResItemDesc, reinterpret_cast<byte*>(&rtefnc)))
				hResult = S_OK;
		}
	}
	return hResult;
}

STDMETHODIMP CPE32PropsView::EnumPE32StringResources(__in LCID lcid, __in DWORD dwID, __in byte* pResData, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PRES_TYPE_ENUM_CONTEXT pRTEC(reinterpret_cast<PRES_TYPE_ENUM_CONTEXT>(pContext));
	CComBSTR String;
	String.Attach(pwszString);
	if (pRTEC && sizeof(RES_TYPE_ENUM_CONTEXT) == pRTEC ->dwStructSize)
	{	
		hResult = S_OK;

		__wchar_t wszItem[TMP_BUFFER_LENGTH_MAX] = { 0 };
		__wchar_t wszLCIDName[LOCALE_NAME_MAX_LENGTH + 1] = { 0 };
		if (!::VerLanguageNameW(lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH))
			if (!::LCIDToLocaleName(lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, 0))
				::StringCchPrintfW(&wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, L"%4X", lcid);

		//	Looking for tree item with that LCID name ...
		HTREEITEM hTILCID(TreeView_GetChild(pRTEC ->hTree, pRTEC ->hti));
		while (hTILCID)
		{
			TVITEMW tviw = { 0 };
			tviw.mask = TVIF_HANDLE | TVIF_PARAM;
			tviw.hItem = hTILCID;
			if (TreeView_GetItem(pRTEC ->hTree, &tviw))
			{
				if (tviw.lParam == lcid)
					break;
				hTILCID = TreeView_GetNextSibling(pRTEC ->hTree, hTILCID);
			}
			else
			{
				hTILCID = reinterpret_cast<HTREEITEM>(-1);
				break;
			}
		}

		if (hTILCID != reinterpret_cast<HTREEITEM>(-1))
		{
			if (!hTILCID)
			{
				CComBSTR LCIDName = wszLCIDName;
				hTILCID = AddItemToTreeView(pRTEC ->hTree, pRTEC ->hti, TVI_LAST, LCIDName, lcid);
			}
			if (!hTILCID)
				hTILCID = pRTEC ->hti;
		}
		else
			hTILCID = pRTEC ->hti;

		//	Add string under it's LCID
		CResString StringPattern(IDS_RES_TYPE_STRING_ID);
		::StringCchPrintfW(&wszItem[0], TMP_BUFFER_LENGTH_MAX, StringPattern.m_bszResString, dwID, dwStringLen, String.m_str);
		CComBSTR Item = wszItem;
		AddItemToTreeView(pRTEC ->hTree, hTILCID, TVI_LAST, Item);		
	}
	return hResult;
}

STDMETHODIMP CPE32PropsView::EnumPE32MsgTableItemsResources(__in LCID lcid, __in DWORD dwMsgID, __in byte* pResData, __in DWORD dwID, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PRES_TYPE_ENUM_CONTEXT pRTEC(reinterpret_cast<PRES_TYPE_ENUM_CONTEXT>(pContext));
	CComBSTR String;
	String.Attach(pwszString);
	if (pRTEC && sizeof(RES_TYPE_ENUM_CONTEXT) == pRTEC ->dwStructSize)
	{	
		hResult = S_OK;

		__wchar_t wszItem[TMP_BUFFER_LENGTH_MAX] = { 0 };
		__wchar_t wszLCIDName[LOCALE_NAME_MAX_LENGTH + 1] = { 0 };
		if (!::VerLanguageNameW(lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH))
			if (!::LCIDToLocaleName(lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, 0))
				::StringCchPrintfW(&wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, L"%4X", lcid);

		//	Looking for tree item with that LCID name ...
		HTREEITEM hTILCID(TreeView_GetChild(pRTEC ->hTree, pRTEC ->hti));
		while (hTILCID)
		{
			TVITEMW tviw = { 0 };
			tviw.mask = TVIF_HANDLE | TVIF_PARAM;
			tviw.hItem = hTILCID;
			if (TreeView_GetItem(pRTEC ->hTree, &tviw))
			{
				if (tviw.lParam == lcid)
					break;
				hTILCID = TreeView_GetNextSibling(pRTEC ->hTree, hTILCID);
			}
			else
			{
				hTILCID = reinterpret_cast<HTREEITEM>(-1);
				break;
			}
		}

		if (hTILCID != reinterpret_cast<HTREEITEM>(-1))
		{
			if (!hTILCID)
			{
				CComBSTR LCIDName = wszLCIDName;
				hTILCID = AddItemToTreeView(pRTEC ->hTree, pRTEC ->hti, TVI_LAST, LCIDName, lcid);
			}
			if (!hTILCID)
				hTILCID = pRTEC ->hti;
		}
		else
			hTILCID = pRTEC ->hti;

		//	Add string under it's LCID
		::StringCchPrintfW(&wszItem[0], TMP_BUFFER_LENGTH_MAX, L"#%X: %s", dwMsgID, String.m_str);
		CComBSTR Item = wszItem;
		AddItemToTreeView(pRTEC ->hTree, hTILCID, TVI_LAST, Item);
	}
	return hResult;
}

bool CALLBACK CPE32PropsView::EnumGroupIconTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;
		__wchar_t szItem[TMP_BUFFER_LENGTH] = { 0 };
		LPGRPICONDIR lpGroupIconDir(reinterpret_cast<LPGRPICONDIR>(pResItemDesc ->lpResItemDataPtr));

		CResString GroupIconPattern(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_GROUPICON_NAME : IDS_RES_TYPE_GROUPICON_ID);
		::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, GroupIconPattern.m_bszResString.m_str, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), lpGroupIconDir ->idCount);
		CComBSTR Item = szItem;
		HTREEITEM hTreeItem(AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Item));

		if (hTreeItem && lpGroupIconDir ->idCount)
		{
			CResString IconItemPattern(IDS_RES_TYPE_GROUPICON_ITEM);
			if (IconItemPattern.m_bszResString.m_str)
			{
				for (WORD i(0); i < lpGroupIconDir ->idCount; ++i)
				{
					::ZeroMemory(szItem, TMP_BUFFER_LENGTH * sizeof(__wchar_t));
					::StringCchPrintfW(&szItem[0], TMP_BUFFER_LENGTH, IconItemPattern.m_bszResString.m_str, lpGroupIconDir ->idEntries[i].nID, lpGroupIconDir ->idEntries[i].bWidth, lpGroupIconDir ->idEntries[i].bHeight ? lpGroupIconDir ->idEntries[i].bHeight : lpGroupIconDir ->idEntries[i].bWidth);
					CComBSTR Item = szItem;
					AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Item);
				}
			}
		}
	}
	return fbResult;
}

bool CALLBACK CPE32PropsView::EnumResourcesByType(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;
		__wchar_t szItem[TMP_BUFFER_LENGTH] = { 0 };

		bool fbTypeLibRecognized(false);
		if ((dwResType & 0x80000000) && sizeof(TYPELIB_HDR) < pResItemDesc ->dwResItemDataSize)
		{
			PTYPELIB_HDR pTLHdr(reinterpret_cast<PTYPELIB_HDR>(pResItemDesc ->lpResItemDataPtr));

			if (pTLHdr && (TYPELIB_VALID_HEADER_FLAG == pTLHdr ->dwValidHeaderFlag || TYPELIB_VALID_HEADER_FLAG2 == pTLHdr ->dwValidHeaderFlag))
			{
				CResString TypeLib(IDS_RES_TYPE_TYPELIB);
				CResString TypeLibEx(IDS_RES_TYPELIB_EX);

				__wchar_t szTmp[TMP_BUFFER_LENGTH] = { 0 };
				::StringCchPrintfW(szTmp, TMP_BUFFER_LENGTH, TypeLibEx.m_bszResString.m_str, TypeLib.m_bszResString.m_str, dwResType);

				if (pRTEFC ->hTree && ::IsWindow(pRTEFC ->hTree))
				{
					CComPtr<ITypeLib> ppvTypeLib;
					CComBSTR PE32ImgFileName;

					if (SUCCEEDED(pRTEFC ->pMainDlg ->mf_pPE32 ->GetFullPE32FileName(&PE32ImgFileName.m_str)))
					{
						__wchar_t ResourceName[2 * MAX_PATH] = { 0 };
						__wchar_t ResourceID[MAX_PATH] = { 0 };

						if (rtByString == pResItemDesc ->ResIDType)
							::StringCchPrintfW(ResourceName, 2 * MAX_PATH, L"%s\\%s", PE32ImgFileName.m_str, pResItemDesc ->pwszName);
						else
							::StringCchPrintfW(ResourceName, 2 * MAX_PATH, L"%s\\%d", PE32ImgFileName.m_str, pResItemDesc ->dwID);

						if (SUCCEEDED(::LoadTypeLib(&ResourceName[0], &ppvTypeLib.p)))
						{
							fbTypeLibRecognized = true;
							TLIBATTR* ptlbAttr(nullptr);
							if (SUCCEEDED(ppvTypeLib ->GetLibAttr(&ptlbAttr)))
							{
								//	Change parent item text ...
								if (pRTEFC ->hti)
								{
									if (GetCommCtl32DllVersion() >= PACKVERSION(4,71))
									{
										TVITEMEXW tvi = { 0 };
										tvi.mask = TVIF_TEXT | TVIF_HANDLE;
										tvi.pszText = szTmp;
										::StringCchLengthW(szTmp, TMP_BUFFER_LENGTH, reinterpret_cast<size_t*>(&tvi.cchTextMax));
										tvi.hItem = pRTEFC ->hti;
										TreeView_SetItem(pRTEFC ->hTree, &tvi);
									}
									else
									{
										TVITEMW tvi = { 0 };
										tvi.mask = TVIF_TEXT | TVIF_HANDLE;
										tvi.pszText = szTmp;
										::StringCchLengthW(szTmp, TMP_BUFFER_LENGTH, reinterpret_cast<size_t*>(&tvi.cchTextMax));
										tvi.hItem = pRTEFC ->hti;
										TreeView_SetItem(pRTEFC ->hTree, &tvi);
									}
								}

								__wchar_t szTypeLib[TMP_BUFFER_LENGTH] = { 0 };

								if (TYPELIB_VALID_HEADER_FLAG == pTLHdr ->dwValidHeaderFlag)
								{
									if (rtByString == pResItemDesc ->ResIDType)
									{
										//	resource id is string
										CResString TypeLibLine(IDS_RES_TYPELIB3);
										::StringCchPrintfW(szTypeLib, TMP_BUFFER_LENGTH, TypeLibLine.m_bszResString.m_str, pResItemDesc ->pwszName, ptlbAttr ->wMajorVerNum /*HIWORD(pTLHdr ->dwVersion)*/, ptlbAttr ->wMinorVerNum /*LOWORD(pTLHdr ->dwVersion)*/, pResItemDesc ->dwResItemDataSize);
									}
									else
									{
										//	resource id is number
										CResString TypeLibLine(IDS_RES_TYPELIB4);
										::StringCchPrintfW(szTypeLib, TMP_BUFFER_LENGTH, TypeLibLine.m_bszResString.m_str, pResItemDesc ->dwID, ptlbAttr ->wMajorVerNum /*HIWORD(pTLHdr ->dwVersion)*/, ptlbAttr ->wMinorVerNum /*LOWORD(pTLHdr ->dwVersion)*/, pResItemDesc ->dwResItemDataSize);
									}
								}
								else
								{
									if (rtByString == pResItemDesc ->ResIDType)
									{
										//	resource id is string
										CResString TypeLibLine(IDS_RES_TYPELIB1);
										::StringCchPrintfW(szTypeLib, TMP_BUFFER_LENGTH, TypeLibLine.m_bszResString.m_str, pResItemDesc ->pwszName, pResItemDesc ->dwResItemDataSize);
									}
									else
									{
										//	resource id is number
										CResString TypeLibLine(IDS_RES_TYPELIB2);
										::StringCchPrintfW(szTypeLib, TMP_BUFFER_LENGTH, TypeLibLine.m_bszResString.m_str, pResItemDesc ->dwID, pResItemDesc ->dwResItemDataSize);
									}
								}

								CComBSTR Item = szTypeLib;
								AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Item);

								//	Add typelib info
								CComBSTR pBstrName;
								CComBSTR pBstrDocString;

								if (SUCCEEDED(ppvTypeLib ->GetDocumentation(-1, &pBstrName.m_str, &pBstrDocString.m_str, nullptr, nullptr)))
								{
									if (pBstrName.m_str)
									{
										AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, pBstrName);
										pBstrName.Empty();
									}
									if (pBstrDocString.m_str)
									{
										AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, pBstrDocString);
										pBstrDocString.Empty();
									}
								}

								__wchar_t szTLGUID[TMP_BUFFER_LENGTH] = { 0 };
								if (::StringFromGUID2(ptlbAttr ->guid, szTLGUID, TMP_BUFFER_LENGTH))
								{
									__wchar_t szTypeLibGUID[TMP_LARGE_BUFFER_LENGTH] = { 0 };
									if (SUCCEEDED(::StringCchPrintfW(szTypeLibGUID, TMP_LARGE_BUFFER_LENGTH, L"TypeLib GUID %s", szTLGUID)))
									{
										CComBSTR Line = szTypeLibGUID;
										AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Line);
									}
								}

								ppvTypeLib ->ReleaseTLibAttr(ptlbAttr);
								ppvTypeLib.Release();
							}
						}
					}
				}
			}
		}

		if (!fbTypeLibRecognized)
		{
			CResString ResTemplate(rtByString == pResItemDesc ->ResIDType ? IDS_RES_TYPE_NAME : IDS_RES_TYPE_ID);
			::StringCchPrintfW(szItem, TMP_BUFFER_LENGTH, ResTemplate.m_bszResString, rtByString == pResItemDesc ->ResIDType ? pResItemDesc ->pwszName : reinterpret_cast<BSTR>(pResItemDesc ->dwID), pResItemDesc ->dwResItemDataSize);

			//	Add item to mf_UnkResItem
			int nIndex(DSA_ERR);
			if (pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA)
			{
				UNK_RES_ITEM uri = { 0 };
				uri.cbStructSize = sizeof(UNK_RES_ITEM);
				uri.dwResType = dwResType;
				::CopyMemory(&uri.Desc, pResItemDesc, sizeof(__PEIMGRESOURCEITEM));

				nIndex = ::DSA_InsertItem(pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA, DSA_APPEND, &uri);
			}

			CComBSTR Item = szItem;
			AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Item, nIndex + 1);
		}
	}
	return fbResult;
}

bool CALLBACK CPE32PropsView::EnumManifestResourcesByType(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;

		CComPtr<IXMLDOMDocument3> pDoc;
		if (FAILED(pDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER)))
		{
			if (FAILED(pDoc.CoCreateInstance(__uuidof(DOMDocument40), NULL, CLSCTX_INPROC_SERVER)))
			{
				if (FAILED(pDoc.CoCreateInstance(__uuidof(DOMDocument30), NULL, CLSCTX_INPROC_SERVER)))
					pDoc.p = nullptr;
			}
		}

		CComPtr<IStream> pStream;
		if (SUCCEEDED(::CreateStreamOnHGlobal(nullptr, TRUE, reinterpret_cast<LPSTREAM*>(&pStream.p))))
		{
			ULONG nWritten(0);
			if (SUCCEEDED(pStream ->Write(pResItemDesc ->lpResItemDataPtr, pResItemDesc ->dwResItemDataSize, &nWritten)))
			{
				STATSTG stat = { 0 };
				if (SUCCEEDED(pStream ->Stat(&stat, STATFLAG_NONAME)) && stat.cbSize.LowPart != pResItemDesc ->dwResItemDataSize)
					pStream.Release();

				if (stat.cbSize.HighPart == 0 && stat.cbSize.LowPart == pResItemDesc ->dwResItemDataSize)
				{
					//	Change stream pointer to beginning ...
					LARGE_INTEGER liOffset;
					liOffset.QuadPart = 0;
					ULARGE_INTEGER ulNewPointer;
					if (FAILED(pStream ->Seek(liOffset, STREAM_SEEK_SET, &ulNewPointer)))
						pStream.Release();
				}
			}
			else
				pStream.Release();
		}


		if (pDoc.p && pStream.p)
		{
			pDoc ->put_async(VARIANT_FALSE);  
			pDoc ->put_validateOnParse(VARIANT_FALSE);
			pDoc ->put_resolveExternals(VARIANT_FALSE);
			pDoc ->put_preserveWhiteSpace(VARIANT_TRUE);

			VARIANT_BOOL retval(VARIANT_TRUE);
			VARIANT xmlSource;
			::VariantInit(&xmlSource);
			xmlSource.vt = VT_UNKNOWN;
			xmlSource.punkVal = pStream.p;

			HRESULT hResult;
			if (S_OK == (hResult = pDoc ->load(xmlSource, &retval)) && VARIANT_TRUE == retval) 
			{
				CComPtr<IXMLDOMElement> pXMLDocument;
				if (SUCCEEDED(pDoc ->get_documentElement(&pXMLDocument.p)))
				{

					VARIANT_BOOL vbParsed(VARIANT_TRUE);
					if (SUCCEEDED(pXMLDocument ->get_parsed(&vbParsed)) && VARIANT_TRUE == vbParsed)
					{
						CComPtr<IXMLDOMNode> pRootNode;
						if (SUCCEEDED(pXMLDocument ->get_parentNode(&pRootNode.p)))
						{
							VARIANT_BOOL vbHasChilds(VARIANT_TRUE);
							if (SUCCEEDED(pRootNode ->hasChildNodes(&vbHasChilds)) && VARIANT_TRUE == vbHasChilds)
							{
								CComPtr<IXMLDOMNodeList> pChildNodes;
								if (SUCCEEDED(pRootNode ->get_childNodes(&pChildNodes.p)))
								{
									long nChildListLen(0);
									if (SUCCEEDED(pChildNodes ->get_length(&nChildListLen)) && nChildListLen)
									{
										CComPtr<IXMLDOMNode> pNode;
										for (long i(0); i < nChildListLen; ++i)
										{
											if (SUCCEEDED(pChildNodes ->get_item(i, &pNode.p)))
											{
												DisplayNode(pNode.p, pRTEFC ->hTree, pRTEFC ->hti);
												pNode.Release();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return fbResult;
}

void CPE32PropsView::DisplayNode(__in IXMLDOMNode* pNode, __in HWND hTree, __in HTREEITEM hti)
{
	CComPtr<IXMLDOMNode> pvNode(pNode);
	DOMNodeType NodeType;
	if (SUCCEEDED(pvNode ->get_nodeType(&NodeType)) && (NODE_ELEMENT == NodeType || NODE_PROCESSING_INSTRUCTION == NodeType))
	{
		CComBSTR bstrNodeName;
		HTREEITEM htiNode(hti);
		if (SUCCEEDED(pNode ->get_nodeName(&bstrNodeName.m_str)))
			htiNode = AddItemToTreeView(hTree, hti, TVI_LAST, bstrNodeName);

		VARIANT vNodeValue;
		if (SUCCEEDED(pNode ->get_nodeValue(&vNodeValue)))
		{
			__wchar_t* pwszValue;
			if (SUCCEEDED(::VariantGetStringElem(vNodeValue, 0, &pwszValue)) && pwszValue)
			{
				CComBSTR NodeValue = pwszValue;
				AddItemToTreeView(hTree, htiNode, TVI_LAST, bstrNodeName);
			}
		}

		CComPtr<IXMLDOMNamedNodeMap> pAttributes;
		if (SUCCEEDED(pNode ->get_attributes(&pAttributes.p)) && pAttributes.p)
		{
			long nAttrLen(0);
			if (SUCCEEDED(pAttributes ->get_length(&nAttrLen)) && nAttrLen)
			{
				CComPtr<IXMLDOMNode> pAttribute;
				CResString ManifestElementAttributes(IDS_RES_MANIFEST_ATTRIBUTES);
				HTREEITEM htiAttr(AddItemToTreeView(hTree, htiNode, TVI_LAST, ManifestElementAttributes.m_bszResString));
				HTREEITEM htiAttrName(htiAttr);
				for (long i(0); i < nAttrLen; ++i)
				{
					if (SUCCEEDED(pAttributes ->get_item(i, &pAttribute.p)))
					{
						CComBSTR AttrName;
						if (SUCCEEDED(pAttribute ->get_nodeName(&AttrName.m_str)))
							htiAttrName = AddItemToTreeView(hTree, htiAttr, TVI_LAST, AttrName);

						VARIANT vAttrValue;
						if (SUCCEEDED(pAttribute ->get_nodeValue(&vAttrValue)))
						{
							__wchar_t* pwszValue;
							if (SUCCEEDED(::VariantGetStringElem(vAttrValue, 0, &pwszValue)) && pwszValue)
							{
								CComBSTR AttrValue = pwszValue;
								AddItemToTreeView(hTree, htiAttrName, TVI_LAST, AttrValue);
							}
						}
						pAttribute.Release();
					}
				}
			}
		}

		VARIANT_BOOL vbHasChilds(VARIANT_FALSE);
		if (SUCCEEDED(pvNode ->hasChildNodes(&vbHasChilds)) && VARIANT_TRUE == vbHasChilds)
		{
			CComPtr<IXMLDOMNodeList> pChildNodes;
			if (SUCCEEDED(pvNode ->get_childNodes(&pChildNodes.p)))
			{
				long nChildListLen(0);
				if (SUCCEEDED(pChildNodes ->get_length(&nChildListLen)) && nChildListLen)
				{
					CComPtr<IXMLDOMNode> pNode;
					for (long i(0); i < nChildListLen; ++i)
					{
						if (SUCCEEDED(pChildNodes ->get_item(i, &pNode.p)))
						{
							DisplayNode(pNode.p, hTree, htiNode);
							pNode.Release();
						}
					}
				}
			}
		}
	}
}

bool CALLBACK CPE32PropsView::EnumPEImgVersionInfo(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext)
{
	bool fbResult(false);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		fbResult = true;

		__wchar_t wszLCIDName[LOCALE_NAME_MAX_LENGTH + 1] = { 0 };
		if (!::VerLanguageNameW(pResItemDesc ->lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH))
			if (!::LCIDToLocaleName(pResItemDesc ->lcid, &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, 0))
				::StringCchPrintfW(&wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, L"%4X", pResItemDesc ->lcid);

		int nIndex(DSA_ERR);
		if (pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA)
		{
			UNK_RES_ITEM uri = { 0 };
			uri.cbStructSize = sizeof(UNK_RES_ITEM);
			uri.dwResType = dwResType;
			::CopyMemory(&uri.Desc, pResItemDesc, sizeof(__PEIMGRESOURCEITEM));

			nIndex = ::DSA_InsertItem(pRTEFC ->pMainDlg ->mf_UnkResItem.mf_HDSA, DSA_APPEND, &uri);
		}

		CComBSTR Lcid = wszLCIDName;
		HTREEITEM hTreeItemLCID(AddItemToTreeView(pRTEFC ->hTree, pRTEFC ->hti, TVI_LAST, Lcid, nIndex + 1));

		VS_VERSIONINFO* pVI(reinterpret_cast<VS_VERSIONINFO*>(pResItemDesc ->lpResItemDataPtr));
		DWORD dwVersionResSize(pResItemDesc ->dwResItemDataSize);
		VS_FIXEDFILEINFO* ppFFI(nullptr);
		WORD wnFFILen(0);

		if (S_OK == pRTEFC ->pMainDlg ->mf_pPE32 ->IsVersionInfoValid(pVI))
		{
			WORD* pTmp(&pVI ->Padding1[0]);
			if (E_PE32VIEW_SUCCESS == pRTEFC ->pMainDlg ->mf_pPE32 ->GetPEImgVersionFixedFileInfo(pVI, &ppFFI, &wnFFILen))
			{
				CResString FixedFileInfo(IDS_RES_VER_FIXED_FILE_INFO);
				HTREEITEM hTreeItem(AddItemToTreeView(pRTEFC ->hTree, hTreeItemLCID, TVI_LAST, FixedFileInfo.m_bszResString));

				CComBSTR Line;
			
				FormatResource(Line, IDS_RES_VER_FIXED_FILE_INFO_SIG, ppFFI ->dwSignature);
				AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line);

				FormatResource(Line, IDS_RES_VER_FIXED_FILE_INFO_VER, HIBYTE(HIWORD(ppFFI ->dwStrucVersion)), LOBYTE(HIWORD(ppFFI ->dwStrucVersion)), HIBYTE(LOWORD(ppFFI ->dwStrucVersion)), LOBYTE(LOWORD(ppFFI ->dwStrucVersion)));
				AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line);

				FormatResource(Line, IDS_RES_VER_FIXED_FILE_INFO_FILE_VER, HIBYTE(LOWORD(ppFFI ->dwFileVersionMS)), LOBYTE(LOWORD(ppFFI ->dwFileVersionMS)), HIBYTE(LOWORD(ppFFI ->dwFileVersionLS)), LOBYTE(LOWORD(ppFFI ->dwFileVersionLS)));
				AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line);

				FormatResource(Line, IDS_RES_VER_FIXED_FILE_INFO_PROD_VER, HIBYTE(LOWORD(ppFFI ->dwProductVersionMS)), LOBYTE(LOWORD(ppFFI ->dwProductVersionMS)), HIBYTE(LOWORD(ppFFI ->dwProductVersionLS)), LOBYTE(LOWORD(ppFFI ->dwProductVersionLS)));
				AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line);
			
				DWORD dwFlags(ppFFI ->dwFileFlags & ppFFI ->dwFileFlagsMask);
				if (dwFlags)
				{
					CResString FileFlags(IDS_RES_VER_FIXED_FILE_INFO);
					HTREEITEM htiTmp(AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line));
					DWORD dwID(IDS_RES_VER_FIXED_FILE_INFO_NORMAL);

					switch(dwFlags)
					{
					case VS_FF_DEBUG:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_DEBUG;
						break;
					case VS_FF_PRERELEASE:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_PRERELEASE;
						break;
					case VS_FF_PATCHED:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_PATCHED;
						break;
					case VS_FF_PRIVATEBUILD:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_PRIVATEBUILD;
						break;
					case VS_FF_INFOINFERRED:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_INFOINFERRED;
						break;
					case VS_FF_SPECIALBUILD:
						dwID = IDS_RES_VER_FIXED_FILE_INFO_SPECIALBUILD;
						break;
					}
					CResString FileProp(dwID);
					AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileProp.m_bszResString);
				}

				if (ppFFI ->dwFileOS)
				{
					CResString TargetSystem(IDS_RES_VER_FIXED_FILE_TARGET_OS);
					HTREEITEM htiTmp(AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, TargetSystem.m_bszResString));

					switch (ppFFI ->dwFileOS)
					{
					case VOS_DOS:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_DOS);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_OS216:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_OS216);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_OS232:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_OS232);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_NT:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_NT);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_WINCE:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_WINCE);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS__WINDOWS16:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS__WINDOWS16);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS__PM16:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS__PM16);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS__PM32:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS__PM32);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS__WINDOWS32:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS__WINDOWS32);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_OS216_PM16:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_OS216_PM16);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_OS232_PM32:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_OS232_PM32);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					case VOS_NT_WINDOWS32:
						{
							CResString FileSystem(IDS_RES_VER_FIXED_FILE_VOS_NT_WINDOWS32);
							AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, FileSystem.m_bszResString);
						}
						break;
					}
				}
				else
				{
					CResString TargetSystem(IDS_RES_VER_FIXED_FILE_TARGET_OS);
					HTREEITEM htiTmp(AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, TargetSystem.m_bszResString));

					CResString TargetSystemUnknown(IDS_RES_VER_FIXED_FILE_VOS_UNKNOWN);
					AddItemToTreeView(pRTEFC ->hTree, htiTmp, TVI_LAST, TargetSystemUnknown.m_bszResString);
				}

				CResString FileType(IDS_RES_VER_FIXED_FILE_FILE_TYPE);
				HTREEITEM htiFileType(AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, FileType.m_bszResString));

				switch(ppFFI ->dwFileType)
				{
				case VFT_UNKNOWN:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_UNKNOWN);
						AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString);
					}
					break;
				case VFT_APP:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_APP);
						AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString);
					}
					break;
				case VFT_DLL:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_DLL);
						AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString);
					}
					break;
				case VFT_DRV:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_DRV);
						HTREEITEM htiFileTypeVal(AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString));

						DWORD dwID(IDS_RES_VER_FIXED_FILE_VFT2_UNKNOWN);
						switch (ppFFI ->dwFileSubtype)
						{
						case VFT2_DRV_PRINTER:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_PRINTER;
							break;
						case VFT2_DRV_KEYBOARD:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_KEYBOARD;
							break;
						case VFT2_DRV_LANGUAGE:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_LANGUAGE;
							break;
						case VFT2_DRV_DISPLAY:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_DISPLAY;
							break;
						case VFT2_DRV_MOUSE:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_MOUSE;
							break;
						case VFT2_DRV_NETWORK:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_NETWORK;
							break;
						case VFT2_DRV_SYSTEM:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_SYSTEM;
							break;
						case VFT2_DRV_INSTALLABLE:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_INSTALLABLE;
							break;
						case VFT2_DRV_SOUND:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_SOUND;
							break;
						case VFT2_DRV_COMM:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_COMM;
							break;
						case VFT2_DRV_INPUTMETHOD:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_INPUTMETHOD;
							break;
						case VFT2_DRV_VERSIONED_PRINTER:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_DRV_VERSIONED_PRINTER;
							break;
						}
						CResString FileSubTypeTmp(dwID);
						AddItemToTreeView(pRTEFC ->hTree, htiFileTypeVal, TVI_LAST, FileSubTypeTmp.m_bszResString);
					}
					break;
				case VFT_FONT:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_FONT);
						HTREEITEM htiFont(AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString));

						DWORD dwID(IDS_RES_VER_FIXED_FILE_VFT2_FONT_UNKNOWN);
						switch(ppFFI ->dwFileSubtype)
						{
						case VFT2_FONT_RASTER:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_FONT_RASTER;
							break;
						case VFT2_FONT_VECTOR:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_FONT_VECTOR;
							break;
						case VFT2_FONT_TRUETYPE:
							dwID = IDS_RES_VER_FIXED_FILE_VFT2_FONT_TRUETYPE;
							break;
						}

						CResString FontSubTypeTmp(dwID);
						AddItemToTreeView(pRTEFC ->hTree, htiFont, TVI_LAST, FontSubTypeTmp.m_bszResString);
					}
					break;

				case VFT_VXD:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_VXD);
						AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString);
					}
					break;
				case VFT_STATIC_LIB:
					{
						CResString FileTypeTmp(IDS_RES_VER_FIXED_FILE_VFT_STATIC_LIB);
						AddItemToTreeView(pRTEFC ->hTree, htiFileType, TVI_LAST, FileTypeTmp.m_bszResString);
					}
					break;
				}

				if (ppFFI ->dwFileDateMS || ppFFI ->dwFileDateLS)
				{
					ULARGE_INTEGER t;
					t.HighPart = ppFFI ->dwFileDateMS;
					t.LowPart = ppFFI ->dwFileDateLS;

					struct tm _tm = { 0 };
					gmtime(reinterpret_cast<time_t*>(&t), &_tm);

					CComBSTR Line;
					FormatResource(Line, IDS_RES_VER_FIXED_FILE_DATE_TIME_STAMP, _tm.tm_mday, _tm.tm_mon, _tm.tm_year, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
					AddItemToTreeView(pRTEFC ->hTree, hTreeItem, TVI_LAST, Line);
				}
			}

			PLANGANDCODEPAGE lpTranslate(nullptr);
			WORD cnTranslate(0);

			if (E_PE32VIEW_SUCCESS == pRTEFC ->pMainDlg ->mf_pPE32 ->GetPEImgVersionTranslations(pVI, &lpTranslate, &cnTranslate))
			{
				__wchar_t wszLCIDName[LOCALE_NAME_MAX_LENGTH + 1] = { 0 };

				RES_TYPE_ENUM_VERSION_STRINGS rtvs = { 0 };
				rtvs.dwStructSize = sizeof(RES_TYPE_ENUM_VERSION_STRINGS);
				rtvs.pMainDlg = pRTEFC ->pMainDlg;
				rtvs.hTree = pRTEFC ->hTree;

				for (WORD iTemp(0); iTemp < cnTranslate; ++iTemp)
				{
					if (!::VerLanguageNameW(MAKELONG(lpTranslate[iTemp].wLanguage, lpTranslate[iTemp].wCodePage), &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH))
						if (!::LCIDToLocaleName(MAKELONG(lpTranslate[iTemp].wLanguage, lpTranslate[iTemp].wCodePage), &wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, 0))
							::StringCchPrintfW(&wszLCIDName[0], LOCALE_NAME_MAX_LENGTH, L"%4X", MAKELONG(lpTranslate[iTemp].wLanguage, lpTranslate[iTemp].wCodePage));

					CComBSTR LangName = wszLCIDName;
					HTREEITEM hTreeItem(AddItemToTreeView(pRTEFC ->hTree, hTreeItemLCID, TVI_LAST, LangName));
					if (hTreeItem)
					{
						rtvs.hTreeRoot = hTreeItem;

						if (FAILED(pRTEFC ->pMainDlg ->mf_pPE32 ->EnumPEImgVersionStringTableStrings(pVI, lpTranslate[iTemp].wLanguage, lpTranslate[iTemp].wCodePage, dynamic_cast<IPE32VersionCallback*>(rtvs.pMainDlg), &rtvs)))
							break;
					}
				}
			}
		}
		else
		{
			CResString VS_VERSIONINFO_Invalid(IDS_RES_VER_FILE_VER_INFO_INVALID);
			AddItemToTreeView(pRTEFC ->hTree, hTreeItemLCID, TVI_LAST, VS_VERSIONINFO_Invalid.m_bszResString);
		}
	}
	return fbResult;
}

INT_PTR CALLBACK CPE32PropsView::ChildTypeLibViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR iResult(0);
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hMainDlg(::GetParent(hDlg));

			__wchar_t pszWindowText[MAX_PATH] = { 0 };
			::GetWindowTextW(hMainDlg, pszWindowText, MAX_PATH);

			HWND hTab(::GetDlgItem(hMainDlg, IDC_TAB_TYPELIB));

			CComBSTR TypeLibResName;
			TypeLibResName.Attach(reinterpret_cast<BSTR>(lParam));

			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(::GetParent(hDlg), lpwszTypeLibDlgProp)));

			if (TypeLibResName.m_str && pView)
			{	
				HWND hTree(::GetDlgItem(hDlg, IDC_TREE_TYPELIB));

				if (lParam && hTree && ::IsWindow(hTree))
				{
					CComPtr<ITypeLib> TypeLib;

					if (SUCCEEDED(::LoadTypeLibEx(TypeLibResName.m_str, REGKIND_NONE, &TypeLib.p)))
					{
						UINT n, m, nCount(TypeLib ->GetTypeInfoCount());
						CComPtr<ITypeInfo> iInfo;
						TYPEATTR *pTypeAttr;
						FUNCDESC *pFuncInfo;
						CComBSTR strName;
						HTREEITEM hNode, hFunction;

						__wchar_t wchBuffer[TMP_BUFFER_LENGTH_MAX];

						for (n = 0; n < nCount; ++n)
						{
							::ZeroMemory(wchBuffer, TMP_BUFFER_LENGTH_MAX * sizeof(__wchar_t));

							if (FAILED(TypeLib ->GetTypeInfo(n, &iInfo.p)) || !iInfo.p)
								continue;

							if (FAILED(iInfo ->GetTypeAttr(&pTypeAttr)))
							{
								iInfo.Release();
								continue;
							}

							TypeLib ->GetDocumentation(n, &strName.m_str, nullptr, nullptr, nullptr);

							if (strName.m_str)
								::StringCchCopyW(wchBuffer, TMP_BUFFER_LENGTH_MAX, strName.m_str);

							if (TKIND_ENUM != pTypeAttr->typekind)
							{
								CComBSTR strGuid(pTypeAttr ->guid);
								strGuid.ToUpper();

								::StringCchCatW(wchBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
								::StringCchCatW(wchBuffer, TMP_BUFFER_LENGTH_MAX, strGuid.m_str);								
							}

							{
								__wchar_t wchBuffer1[TMP_BUFFER_LENGTH_MAX] = { 0 };

								switch (pTypeAttr->typekind)
								{
								case TKIND_ENUM: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Enumeration"); 
									break;
								case TKIND_RECORD: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Record"); 
									break;
								case TKIND_MODULE: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Module"); 
									break;
								case TKIND_INTERFACE: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Interface"); 
									break;
								case TKIND_DISPATCH: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Dispatch interface"); 
									break;
								case TKIND_COCLASS: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Coclass");
									break;
								case TKIND_ALIAS: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Alias");
									break;
								case TKIND_UNION: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Union"); 
									break;
								default: 
									::StringCchCopyW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, L"Unknown type");
									break;
								}

								::StringCchCatW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
								::StringCchCatW(wchBuffer1, TMP_BUFFER_LENGTH_MAX, wchBuffer);
								::CopyMemory(wchBuffer, wchBuffer1, TMP_BUFFER_LENGTH_MAX * sizeof(__wchar_t));
							}

							TVINSERTSTRUCTW tv_ins = { 0 };
							TVITEMW tvi = { 0 }; 

							tvi.mask = TVIF_TEXT | TVIF_PARAM;
							tvi.lParam = (pTypeAttr->typekind == TKIND_INTERFACE || pTypeAttr->typekind == TKIND_DISPATCH) ? 3 : 0;
							::StringCchLengthW(wchBuffer, TMP_BUFFER_LENGTH_MAX, reinterpret_cast<size_t*>(&tvi.cchTextMax));
							tvi.pszText = wchBuffer;

							tv_ins.item = tvi;
							tv_ins.hParent = TVI_ROOT;
							tv_ins.hInsertAfter = TVI_LAST;							

							hNode = TreeView_InsertItem(hTree, &tv_ins);

							if (pTypeAttr->typekind == TKIND_INTERFACE || pTypeAttr->typekind == TKIND_DISPATCH)
							{
								for (m = 0; m < static_cast<UINT>(pTypeAttr ->cFuncs); ++m)
								{
									if (FAILED(iInfo ->GetFuncDesc(m, &pFuncInfo)))
										break;

									__wchar_t wsNode[TMP_BUFFER_LENGTH_MAX] = { 0 };

									DumpFunc(wsNode, iInfo.p, pFuncInfo, pTypeAttr, pFuncInfo->memid);

									::ZeroMemory(&tv_ins, sizeof(TVINSERTSTRUCTW));
									::ZeroMemory(&tvi, sizeof(TVITEMW));

									tvi.mask = TVIF_TEXT | TVIF_PARAM;
									tvi.lParam = pFuncInfo ->invkind;
									::StringCchLengthW(wsNode, TMP_BUFFER_LENGTH_MAX, reinterpret_cast<size_t*>(&tvi.cchTextMax));
									tvi.pszText = wsNode;

									tv_ins.item = tvi;
									tv_ins.hParent = hNode;
									tv_ins.hInsertAfter = TVI_LAST;

									hFunction = TreeView_InsertItem(hTree, &tv_ins);

									iInfo ->ReleaseFuncDesc(pFuncInfo);
									pFuncInfo = nullptr;
								}
							}
							iInfo ->ReleaseTypeAttr(pTypeAttr);
							pTypeAttr = nullptr;
							iInfo.Release();
						}
						iResult = 1;
					}
				}

				if (::IsThemeActive() && ::IsAppThemed())
				{
					::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
					::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
					::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
				}

				::MoveWindow(hDlg, pView ->mf_rcTLDisplayRect.left, pView ->mf_rcTLDisplayRect.top, pView ->mf_rcTLDisplayRect.right - pView ->mf_rcTLDisplayRect.left, pView ->mf_rcTLDisplayRect.bottom - pView ->mf_rcTLDisplayRect.top, FALSE);
	
				RECT rcClient;
				::GetClientRect(hDlg, &rcClient);
				--rcClient.bottom;
				--rcClient.right;
				::MoveWindow(hTree, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
			}
		}
	}
	return iResult;
}

void CPE32PropsView::DumpFunc(__inout __wchar_t* pwstrNode, __in ITypeInfo* pti, __in FUNCDESC* pfuncdesc, __in TYPEATTR* pattr, __in MEMBERID memid)
{
	HRESULT hr(S_OK);
	CComBSTR bstrName;
	CComBSTR bstrDoc;
	DWORD dwHelpID;

	__wchar_t TmpBuffer[TMP_BUFFER_LENGTH_MAX] = { 0 };

	if (!pwstrNode || !pti || !pfuncdesc || !pattr)
		return;
		
	BOOL fAttributes(FALSE);
	BOOL fAttribute(FALSE);

	if (pattr ->typekind == TKIND_DISPATCH)
	{
		fAttributes = TRUE;
		fAttribute = TRUE;
		__wchar_t Buffer[MAX_PATH] = { 0 };
		::StringCchPrintfW(Buffer, MAX_PATH, L"[ID(%d)", memid);
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
	}
	else if (pattr ->typekind == TKIND_MODULE)
	{
		fAttributes = TRUE;
		fAttribute = TRUE;
		__wchar_t Buffer[MAX_PATH] = { 0 };
		::StringCchPrintfW(Buffer, MAX_PATH, L"[Entry(%d)", memid);
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
	}
	else
		// if there are some attributes
		if ((pfuncdesc ->invkind > 1)|| pfuncdesc ->wFuncFlags ||
			pfuncdesc ->cParamsOpt == -1)
		{
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"[");
			fAttributes = TRUE;
		}
			
	if (pfuncdesc ->invkind & INVOKE_PROPERTYGET)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"propget");
	}

	if (pfuncdesc ->invkind & INVOKE_PROPERTYPUT)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"propput");
	}

	if (pfuncdesc ->invkind & INVOKE_PROPERTYPUTREF)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"propputref");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FRESTRICTED)
	{

		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"restricted");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FSOURCE)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"source");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FBINDABLE)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"bindable");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FREQUESTEDIT)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"requestedit");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FDISPLAYBIND)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"displaybind");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FDEFAULTBIND)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"defaultbind");
	}

	if (pfuncdesc ->wFuncFlags & FUNCFLAG_FHIDDEN)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"hidden");
	}

	if (pfuncdesc ->cParamsOpt == -1)
	{
		if (fAttribute)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

		fAttribute = TRUE;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"vararg");
	}
		
	if (SUCCEEDED(pti ->GetDocumentation(memid, &bstrName.m_str, &bstrDoc.m_str, &dwHelpID, nullptr)))
	{
		if (bstrDoc.m_str)
		{
			if (!fAttributes) 
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"[");
			fAttributes = TRUE;
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);

			__wchar_t Buffer[TMP_BUFFER_LENGTH_MAX] = { 0 };
			::StringCchPrintfW(Buffer, TMP_BUFFER_LENGTH_MAX, L"helpstring(\"%s\")", bstrDoc.m_str);

			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
			if (dwHelpID)
			{
				__wchar_t Buffer[MAX_PATH] = { 0 };
				::StringCchPrintfW(Buffer, MAX_PATH, L"[Entry(%d)", memid);
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
			}
		}
		else if (dwHelpID)
		{
			if (!fAttributes) 
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"[");
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			fAttributes = TRUE;

			__wchar_t Buffer[MAX_PATH] = { 0 };
			::StringCchPrintfW(Buffer, MAX_PATH, L"helpcontext(%#08.8x)", memid);

			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
		}
	}

	if (fAttributes)
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"] ");

	// Write return type
	CComBSTR strTmp;

	TYPEDESCtoString(pti, &pfuncdesc ->elemdescFunc.tdesc, &strTmp.m_str);

	if (strTmp.m_str)
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, strTmp.m_str);
	::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);

	if (pattr ->typekind != TKIND_DISPATCH)
	{   
		// Write calling convention
		switch(pfuncdesc ->callconv)
		{
		case CC_CDECL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_cdecl "); 
			break;
		case CC_PASCAL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_pascal "); 
			break;
		case CC_MACPASCAL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_macpascal "); 
			break;
		case CC_STDCALL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_stdcall "); 
			break;
		case CC_SYSCALL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_syscall "); 
			break;
		case CC_MPWCDECL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_mpwcdecl "); 
			break;
		case CC_MPWPASCAL: 
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"_mpwpascal "); 
			break;
		}
	}

	UINT cNames(0);
	BSTR rgbstrNames[MAX_NAMES] = { 0 };
	CComBSTR Names[MAX_NAMES];

	if (FAILED(hr = pti ->GetNames(pfuncdesc ->memid, rgbstrNames, MAX_NAMES, &cNames)))
		return;
		
	if (static_cast<int>(cNames) < static_cast<int>(pfuncdesc->cParams + 1))
	{
		while (static_cast<int>(cNames) < static_cast<int>(pfuncdesc->cParams + 1))
		{
			rgbstrNames[cNames] = ::SysAllocString(OLESTR("rhs"));
			++cNames;
		}
	}

	for (UINT uIndex(0); uIndex < MAX_NAMES; ++uIndex)
		Names[uIndex].Attach(rgbstrNames[uIndex]);
		
	::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Names[0].m_str);
	::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"(");
		
	for (SHORT n(0); n < pfuncdesc->cParams; ++n)
	{
		fAttributes = FALSE;
		fAttribute = FALSE;
			
		if (pfuncdesc ->lprgelemdescParam[n].idldesc.wIDLFlags)
		{
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"[");
			fAttributes = TRUE;
		}
			
		if (pfuncdesc ->lprgelemdescParam[n].idldesc.wIDLFlags & IDLFLAG_FIN)
		{
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"in");
			fAttribute = TRUE;
		}

		if (pfuncdesc ->lprgelemdescParam[n].idldesc.wIDLFlags & IDLFLAG_FOUT)
		{
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"out");
			fAttribute = TRUE;
		}

		if (pfuncdesc ->lprgelemdescParam[n].idldesc.wIDLFlags & IDLFLAG_FLCID)
		{
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"lcid");
			fAttribute = TRUE;
		}

		if (pfuncdesc ->lprgelemdescParam[n].idldesc.wIDLFlags & IDLFLAG_FRETVAL)
		{
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"retval");
			fAttribute = TRUE;
		}
			
		if ((pfuncdesc ->cParamsOpt == -1 && n == pfuncdesc ->cParams - 1) || (n > (pfuncdesc ->cParams - pfuncdesc ->cParamsOpt)))
		{
			if (fAttribute)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
			if (!fAttributes)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"[");
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"optional");
			fAttributes = TRUE;
			fAttribute = TRUE;
		}
			
		if (fAttributes)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"] ");
			
		// type
		CComBSTR str;
		if ((pfuncdesc ->lprgelemdescParam[n].tdesc.vt & 0x0FFF) == VT_CARRAY)
		{
			TYPEDESCtoString(pti, &pfuncdesc -> lprgelemdescParam[n].tdesc.lpadesc->tdescElem, &str.m_str);
			// type name[n]
			if (str.m_str)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, str.m_str);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
				
			if (((n + 1) < MAX_NAMES) && Names[n + 1].m_str)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Names[n + 1].m_str);
				
			// Allocate cDims * lstrlen("[123456]")
			for (USHORT n(0); n < pfuncdesc ->lprgelemdescParam[n].tdesc.lpadesc ->cDims; ++n)
			{
				__wchar_t Buffer[MAX_PATH] = { 0 };
				::StringCchPrintfW(Buffer, MAX_PATH, L"[%d]", pfuncdesc ->lprgelemdescParam[n].tdesc.lpadesc ->rgbounds[n].cElements);
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
			}
		}
		else
		{
			TYPEDESCtoString(pti, &pfuncdesc ->lprgelemdescParam[n].tdesc, &str.m_str);
			if (str.m_str)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, str.m_str);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
			if (((n + 1) < MAX_NAMES) && Names[n + 1].m_str)
				::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Names[n + 1].m_str);				
		}
		if (n < pfuncdesc ->cParams - 1)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemDelimiter);
	}
		
	::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L");");
	::StringCchCatW(pwstrNode, TMP_BUFFER_LENGTH_MAX, TmpBuffer);
}

void CPE32PropsView::TYPEDESCtoString(__in ITypeInfo* pti, __in TYPEDESC* ptdesc, __inout BSTR* psbstr)
{
	if (!psbstr)
		return;

	CComBSTR pstr;
	__wchar_t TmpBuffer[TMP_BUFFER_LENGTH_MAX] = { 0 };

	if (ptdesc ->vt == VT_PTR)
	{
		TYPEDESCtoString(pti, ptdesc->lptdesc, &pstr.m_str);
		::StringCchCopyW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, pstr.m_str);
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemRef);
		pstr = TmpBuffer;
		*psbstr = pstr.Detach();
		return;
	}
	
	if ((ptdesc ->vt & 0x0FFF) == VT_CARRAY)
	{
		TYPEDESCtoString(pti, &ptdesc ->lpadesc ->tdescElem, &pstr.m_str);
		::StringCchCopyW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, pstr.m_str);//lpwszTypeLibItemRef
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibItemRef);

		// Allocate cDims * lstrlen("[123456]")
		for (USHORT n(0); n < ptdesc ->lpadesc ->cDims; ++n)
		{
			__wchar_t Buffer[TMP_BUFFER_LENGTH] = { 0 };
			::StringCchPrintfW(Buffer, TMP_BUFFER_LENGTH, L"[%d]", ptdesc ->lpadesc ->rgbounds[n].cElements);
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Buffer);
		}

		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
		pstr = TmpBuffer;
		*psbstr = pstr.Detach();

		return;
	}
	
	if ((ptdesc ->vt & 0x0FFF) == VT_SAFEARRAY)
	{
		TYPEDESCtoString(pti, ptdesc ->lptdesc, &pstr.m_str);

		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"SAFEARRAY(");
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, pstr.m_str);
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L") ");
		
		pstr = TmpBuffer;
		*psbstr = pstr.Detach();

		return;
	}
	
	if (ptdesc ->vt == VT_USERDEFINED)
	{
		if (!pti)
			return;

		CComPtr<ITypeInfo> refType;
		HRESULT hr;
		if (SUCCEEDED(hr = pti->GetRefTypeInfo(ptdesc ->hreftype, &refType.p)) && refType.p)
		{
			CComBSTR Name;
			DWORD dwHelpID;
			if (FAILED(hr = refType ->GetDocumentation(MEMBERID_NIL, &Name.m_str, nullptr, &dwHelpID, nullptr)))
			{
				*psbstr = nullptr;
				return;
			}
			if (Name.m_str)
				::StringCchCopyW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, Name.m_str);
			refType.Release();
		}
		else
		{
			refType.p = nullptr;
			*psbstr = nullptr;
			return;
		}

		refType.p = nullptr;
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);

		pstr = TmpBuffer;
		*psbstr = pstr.Detach();

		return;
	}

	VARTYPE vt(ptdesc ->vt & ~0xF000);
	if (vt <= VT_CLSID)
	{
		GetTypeDesc(vt, &pstr.m_str);
		if (pstr.m_str)
			::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, pstr.m_str);
	}
	else
		::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, L"Bad VARTYPE");

	::StringCchCatW(TmpBuffer, TMP_BUFFER_LENGTH_MAX, lpwszTypeLibSpace);
	pstr = TmpBuffer;
	*psbstr = pstr.Detach();

	return;
}

void CPE32PropsView::GetTypeDesc(__in VARTYPE vt, __inout BSTR* pbstrDesc)
{
	if (!pbstrDesc)
		return;

	CComBSTR pstrDesc;
	pstrDesc.Empty();

	switch (vt)
	{
	case VT_EMPTY: 
		pstrDesc = L"void"; 
		break;
	case VT_NULL: 
		pstrDesc = L"NULL"; 
		break;
	case VT_I2: 
		pstrDesc = L"short"; 
		break;
	case VT_I4: 
		pstrDesc = L"long"; 
		break;
	case VT_R4: 
		pstrDesc = L"single"; 
		break;
	case VT_R8: 
		pstrDesc = L"double"; 
		break;
	case VT_CY: 
		pstrDesc = L"CURRENCY"; 
		break;
	case VT_DATE: 
		pstrDesc = L"DATE"; 
		break;
	case VT_BSTR: 
		pstrDesc = L"BSTR"; 
		break;
	case VT_DISPATCH: 
		pstrDesc = L"IDispatch *"; 
		break;
	case VT_ERROR: 
		pstrDesc = L"SCODE"; 
		break;
	case VT_BOOL: 
		pstrDesc = L"BOOL"; 
		break;
	case VT_VARIANT: 
		pstrDesc = L"VARIANT"; 
		break;
	case VT_UNKNOWN: 
		pstrDesc = L"IUnknown *"; 
		break;
	case VT_I1: 
		pstrDesc = L"char"; 
		break;
	case VT_UI1: 
		pstrDesc = L"unsigned char"; 
		break;
	case VT_UI2: 
		pstrDesc = L"unsigned short"; 
		break;
	case VT_UI4: 
		pstrDesc = L"unsigned long"; 
		break;
	case VT_I8: 
		pstrDesc = L"int64";
		break;
	case VT_UI8: 
		pstrDesc = L"uint64";
		break;
	case VT_INT: 
		pstrDesc = L"int"; 
		break;
	case VT_UINT: 
		pstrDesc = L"unsigned int"; 
		break;
	case VT_VOID: 
		pstrDesc = L"void";
		break;
	case VT_HRESULT: 
		pstrDesc = L"HRESULT";
		break;
	case VT_PTR: 
		pstrDesc = L"void *";
		break;
	case VT_SAFEARRAY: 
		pstrDesc = L"SAFEARRAY";
		break;
	case VT_CARRAY: 
		pstrDesc = L"CARRAY";
		break;
	case VT_USERDEFINED: 
		pstrDesc = L"USER_DEFINED";
		break;
	case VT_LPSTR: 
		pstrDesc = L"LPSTR"; 
		break;
	case VT_LPWSTR: 
		pstrDesc = L"LPWSTR";
		break;
	case VT_FILETIME: 
		pstrDesc = L"FILETIME"; 
		break;
	case VT_BLOB: 
		pstrDesc = L"BLOB";
		break;
	case VT_STREAM: 
		pstrDesc = L"Stream";
		break;
	case VT_STORAGE: 
		pstrDesc = L"Storage";
		break;
	case VT_STREAMED_OBJECT: 
		pstrDesc = L"Streamed object";
		break;
	case VT_STORED_OBJECT: 
		pstrDesc = L"Stored object";
		break;
	case VT_BLOB_OBJECT: 
		pstrDesc = L"BLoB object";
		break;
	case VT_CF: 
		pstrDesc = L"CF"; 
		break;
	case VT_CLSID: 
		pstrDesc = L"CLSID";
		break;
	default: 
		{
			__wchar_t Buffer[TMP_BUFFER_LENGTH] = { 0 };
			::StringCchPrintfW(Buffer, TMP_BUFFER_LENGTH, L"[Unknown type %0#x (%d)] ", vt, vt);
			pstrDesc = Buffer;
		}
		break;
	}
	if (pstrDesc.m_str)
		*pbstrDesc = pstrDesc.Detach();
	else
		*pbstrDesc = nullptr;
}

HWND CPE32PropsView::CreateChildTypeLibDlg(__in HWND hDlg, __in HWND hTab, __in UINT nDlgID)
{
	HWND hTypeLibViewDlg(nullptr);
	if (hDlg && ::IsWindow(hDlg))
	{
		CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszTypeLibDlgProp)));

		if (pView)
		{
			int nCount(DPA_GetPtrCount(pView ->mf_TypeLibRes.mf_hDPA));

			if (static_cast<UINT>(nCount) > nDlgID)
			{
				CComBSTR TypeLibResourceName;
				TypeLibResourceName = reinterpret_cast<BSTR>(::DPA_GetPtr(pView ->mf_TypeLibRes.mf_hDPA, static_cast<UINT_PTR>(nDlgID)));
				hTypeLibViewDlg = ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_TYPELIBUNIT)), hDlg, ChildTypeLibViewDlgInitProc, reinterpret_cast<LPARAM>(TypeLibResourceName.Detach()));
			}
		}
	}
	return hTypeLibViewDlg;
}

STDMETHODIMP CPE32PropsView::EnumPE32Typelib(__in __PEIMGRESTYPELIBITEM* pTypelibDesc, __in byte* pContext)
{
	HRESULT hResult(S_FALSE);
	PRES_TYPE_ENUM_FN_CONTEXT pRTEFC(reinterpret_cast<PRES_TYPE_ENUM_FN_CONTEXT>(pContext));
	if (pRTEFC && sizeof(RES_TYPE_ENUM_FN_CONTEXT) == pRTEFC ->dwStructSize)
	{
		HWND hTab(pRTEFC ->hTree);
		DPA_AppendPtr(pRTEFC ->pMainDlg ->mf_TypeLibRes.mf_hDPA, reinterpret_cast<void*>(pTypelibDesc ->bstrFullName));

		TCITEMW tie = { 0 };
		tie.mask = TCIF_TEXT | TCIF_IMAGE; 
		tie.iImage = -1;
		tie.pszText = pTypelibDesc ->bstrShortName;
		tie.cchTextMax = ::SysStringLen(pTypelibDesc ->bstrShortName);

		TabCtrl_InsertItem(hTab, TabCtrl_GetItemCount(hTab), &tie);

		::SysFreeString(pTypelibDesc ->bstrShortName);
		pTypelibDesc ->bstrShortName = nullptr;

		hResult = E_PE32VIEW_SUCCESS;
	}
	return hResult;
}

INT_PTR CALLBACK CPE32PropsView::ChildTypeLibDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTab(::GetDlgItem(hDlg, IDC_TAB_TYPELIB));

			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTab, VSCLASS_EXPLORER, nullptr);
				::EnableThemeDialogTexture(hTab, ETDT_ENABLETAB);
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rcClient;
			::GetClientRect(hDlg, &rcClient);
			--rcClient.bottom;
			--rcClient.right;
			::MoveWindow(hTab, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);

			TabCtrl_SetPadding(hTab, 2 * ::GetSystemMetrics(SM_CXDLGFRAME), 2 * ::GetSystemMetrics(SM_CYDLGFRAME));

			RES_TYPE_ENUM_CONTEXT rtec;
 			rtec.dwStructSize = sizeof(RES_TYPE_ENUM_CONTEXT);
			rtec.pMainDlg = pView;
			rtec.hDlg = hDlg;
			rtec.hTree = hTab;
			rtec.pfnEnumResByType = nullptr;

			pView ->mf_pPE32 ->EnumTypeLibraries(dynamic_cast<IPE32TypelibCallback*>(pView), &rtec);

			TabCtrl_SetCurSel(hTab, 0);

			pView ->mf_hwndChildTypeLib = nullptr;

			if (!::SetPropW(hDlg, lpwszTypeLibDlgProp, pView))
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
				::CopyRect(&pView ->mf_rcTLDisplayRect, &rcDisplay);

				pView ->mf_hwndChildTypeLib = pView ->CreateChildTypeLibDlg(hDlg, hTab, 0);
				if (pView ->mf_hwndChildTypeLib)
				{
					::SetWindowLongW(pView ->mf_hwndChildTypeLib, GWL_STYLE, ::GetWindowLongW(pView ->mf_hwndChildTypeLib, GWL_STYLE) | WS_VISIBLE);
					::SetWindowPos(pView ->mf_hwndChildTypeLib, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER  | SWP_FRAMECHANGED);
					::SetWindowPos(pView ->mf_hwndChildTypeLib, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE);
					::ShowWindow(pView ->mf_hwndChildTypeLib, SW_SHOWNORMAL);
				}
			}
		}
		return 1;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh(reinterpret_cast<LPNMHDR>(lParam));
			if (pnmh ->idFrom == IDC_TAB_TYPELIB)
			{
				switch(pnmh ->code)
				{
				case TCN_SELCHANGE:
					{
						int iSel(TabCtrl_GetCurSel(::GetDlgItem(hDlg, pnmh ->idFrom)));
						CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszTypeLibDlgProp)));
						if (pView)
						{
							if (pView ->mf_hwndChildTypeLib)
							{
								::DestroyWindow(pView ->mf_hwndChildTypeLib);
								pView ->mf_hwndChildTypeLib = nullptr;
							}

							pView ->mf_hwndChildTypeLib = pView ->CreateChildTypeLibDlg(hDlg, pnmh ->hwndFrom, static_cast<UINT>(iSel));
							if (pView ->mf_hwndChildTypeLib)
							{
								::SetWindowLongW(pView ->mf_hwndChildTypeLib, GWL_STYLE, ::GetWindowLongW(pView ->mf_hwndChildTypeLib, GWL_STYLE) | WS_VISIBLE);
								::SetWindowPos(pView ->mf_hwndChildTypeLib, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
								::SetWindowPos(pView ->mf_hwndChildTypeLib, HWND_TOP, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
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
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(::GetPropW(hDlg, lpwszTypeLibDlgProp)));
			if (pView)
			{
				if (pView ->mf_hwndChildTypeLib)
				{
					::DestroyWindow(pView ->mf_hwndChildTypeLib);
					pView ->mf_hwndChildTypeLib = nullptr;
				}

				::RemovePropW(hDlg, lpwszTypeLibDlgProp);
				int nTabsCount(TabCtrl_GetItemCount(::GetDlgItem(hDlg, IDC_TAB_TYPELIB)));

				int nCount(DPA_GetPtrCount(pView ->mf_TypeLibRes.mf_hDPA));
				if (nCount)
				{
					CComBSTR Utilizer;
					while (0 < DPA_GetPtrCount(pView ->mf_TypeLibRes.mf_hDPA))
					{
						Utilizer.Attach(reinterpret_cast<BSTR>(::DPA_GetPtr(pView ->mf_TypeLibRes.mf_hDPA, 0)));
						Utilizer.Empty();
						::DPA_DeletePtr(pView ->mf_TypeLibRes.mf_hDPA, 0);
					}
				}
			}
		}
		break;
	}
	return 0;
}
