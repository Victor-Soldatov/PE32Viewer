// PE32Viewer.h : Declaration of the CCoPE32Viewer
#pragma once
#include "resource.h"       // main symbols
#include "PE32Viewer_i.h"
#include <shobjidl.h>
#include <shlobj.h>
#include <atlbase.h>

extern HINSTANCE hDLLInstance;

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <uxtheme.h>
#include <vssym32.h>
#pragma comment(lib, "UxTheme.lib")
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include <wincodec.h>
#pragma comment(lib, "WindowsCodecs.lib")

class CPE32PropsView;

typedef DWORD ARGB;

typedef enum __tagPOPUPPARTS
{
    POPUP_CHECK,
    POPUP_TEXT,
    POPUP_SEPARATOR,
    POPUP_MAX
}
POPUPPARTS;

typedef struct __tagMENUITEMDATA
{
    HMENU hMenu;
    int ID;
    SIZE rgPopupSize[POPUP_MAX];					// Dimensions of each menu item component                                       
}
MENUITEMDATA, *PMENUITEMDATA;

typedef struct __tagMENUITEM 
{
    MENUITEMINFO mii;
    MENUITEMDATA* pmid;
    __wchar_t szItemText[MAX_PATH];
}
MENUITEM, *PMENUITEM ;

typedef struct __tagDRAWITEMMETRICS
{
    RECT rcSelection;								// Selection rectangle
    RECT rcGutter;									// Gutter rectangle
    RECT rcCheckBackground;							// Check background rectangle
    RECT rgrc[POPUP_MAX];							// Menu subitem rectangles
}
DRAWITEMMETRICS, *PDRAWITEMMETRICS;

typedef struct __tagWORKER_INIT_PARAMS
{
	DWORD dwStructSize;
	HANDLE hEvent;
	BSTR pszFileName;
	HWND hParentWnd;
	POINT ptInvokePt;
}
WORKER_INIT_PARAMS, *PWORKER_INIT_PARAMS;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

// CCoPE32Viewer

class ATL_NO_VTABLE CCoPE32Viewer :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCoPE32Viewer, &CLSID_CoPE32Viewer>,
	public IPE32Viewer,
	public IContextMenu3,
	public IShellExtInit
{
private:
	__wchar_t mf_szFileName[MAX_PATH + 1];
	__wchar_t mf_szVerbW[MAX_PATH];
	char mf_szVerbA[MAX_PATH];

	HINSTANCE mf_hInstance;
	UINT mf_nCmdID;
	POINT mf_ptPoint;
	HWND mf_hParent;
	HMENU mf_hMenu;
	UINT mf_nMenuItemIndex;

	HTHEME mf_hTheme;
    HWND mf_hwndTheme;
    MARGINS mf_marPopupCheck;						// Popup check margins
    MARGINS mf_marPopupCheckBackground;				// Popup check background margins
    MARGINS mf_marPopupItem;						// Popup item margins
    MARGINS mf_marPopupText;						// Popup text margins
    MARGINS mf_marPopupAccelerator;					// Popup accelerator margins
    SIZE mf_sizePopupCheck;							// Popup check size metric
    SIZE mf_sizePopupSeparator;						// Popup separator size metric
    int mf_iPopupBorderSize;						// Popup border space between item text and accelerator
    int mf_iPopupBgBorderSize;						// Popup border space between item text and gutter
    int mf_cyMarCheckBackground;					// Additional amount of vertical space to add to checkbox
	COLORREF mf_crItemDisabled;
	COLORREF mf_crItemDisabledHot;
	COLORREF mf_crItemHot;
	COLORREF mf_crItemNormal;
	MENUITEMDATA mf_mid;
	
	HBITMAP mf_hBmp;
protected:
	char* LoadAnsiStringEx(__in HINSTANCE hInstance, __in DWORD MessageID, __in LANGID LangID, __in UINT nCodePage = CP_ACP);
	LCID GetLocale(void);
	STDMETHOD(InitializeTheme)(__in HWND hwndTheme);
	void Flush(void);
	void MeasureMenuItem(__in HWND hwndTheme, __inout MENUITEM *pmi, __inout MEASUREITEMSTRUCT *pmis);
	static void ToMeasureSize(const SIZE *psizeDraw, const MARGINS *pmargins, __out LPSIZE psizeMeasure);
	void LayoutMenuItem(__in MENUITEM *pmi, __in DRAWITEMSTRUCT *pdis, __out DRAWITEMMETRICS *pdim);
	static int RectWidth(const RECT &r) { return r.right - r.left; }
	static int RectHeight(const RECT &r) { return r.bottom - r.top; }
	static void ToDrawRect(__in LPCRECT prcMeasure, __in const MARGINS *pmargins, __out LPRECT prcDraw);
	void ThemedDrawMenuItem(__in HWND hwndTheme, __in MENUITEM *pmi, __in DRAWITEMSTRUCT *pdis);
	static POPUPITEMSTATES ToItemStateId(__in UINT uItemState);
	static POPUPCHECKBACKGROUNDSTATES ToCheckBackgroundStateId(__in int iStateId);
	static POPUPCHECKSTATES ToCheckStateId(__in UINT fType, __in int iStateId);
	static BOOL GetMenuItem(__in ULONG_PTR itemData, __out_bcount(sizeof(MENUITEM)) MENUITEM *pItem);
	static bool HasAlpha(__in ARGB *pargb, __in SIZE& sizImage, __in int cxRow);
public:
	CCoPE32Viewer(void);

DECLARE_REGISTRY_RESOURCEID(IDR_PE32VIEWER)

DECLARE_NOT_AGGREGATABLE(CCoPE32Viewer)

BEGIN_COM_MAP(CCoPE32Viewer)
	COM_INTERFACE_ENTRY(IPE32Viewer)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(IContextMenu2)
	COM_INTERFACE_ENTRY(IContextMenu3)
	COM_INTERFACE_ENTRY(IShellExtInit)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		::BufferedPaintInit();
		return S_OK;
	}

	void FinalRelease()
	{
		if (mf_hBmp)
			::DeleteObject(mf_hBmp);
		::BufferedPaintUnInit();
	}

	const __wchar_t* GetTargetFileName(void) const throw()
	{
		return &mf_szFileName[0];
	}

	HWND GetParentWindowHandle(void) const throw()
	{
		return mf_hParent;
	}

	POINT GetInvokePoint(void) const throw()
	{
		return mf_ptPoint;
	}

	static __wchar_t* LoadStringEx(__in HINSTANCE hInstance, __in DWORD MessageID, __in LANGID LangID);
public:
	STDMETHOD(ProcessMenuMessage)(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam, __out LRESULT *plResult);
	static HRESULT Create32BitHBITMAP(__in HDC hdc, __in const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp);
	static HRESULT AddIconToMenuItem(__in IWICImagingFactory *pFactory, __in HICON hIcon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp);
	static HRESULT AddIconToMenuItemNoWIC(__in HICON hIcon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp);
	static HRESULT AddIconToMenuItemGDI(__in HICON hIcon, __in BOOL fAutoDestroy, __out_opt HBITMAP *phbmp);
protected:
	static void InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp);
	static HRESULT ConvertToPARGB32(__in HDC hdc, __inout ARGB *pargb, __in HBITMAP hbmp, __in SIZE& sizImage, __in int cxRow);
	static HRESULT ConvertBufferToPARGB32(__in HPAINTBUFFER hPaintBuffer, __in HDC hdc, __in HICON hicon, __in SIZE& sizIcon);
	static void CALLBACK PropertiesShowWorker(__inout PTP_CALLBACK_INSTANCE Instance,  __inout_opt  PWORKER_INIT_PARAMS pInitParams);
public:
	//	IPE32Viewer
	STDMETHOD(GetVersion)(__out LONG* plVersion);
	STDMETHOD(GetVersionEx)(__out PE32VIEW_VERSION* psVersionEx);

	//	IShellExtInit
	STDMETHOD(Initialize)(__in_opt PCIDLIST_ABSOLUTE pidlFolder, __in_opt IDataObject *pdtobj, __in_opt HKEY hkeyProgID);

	//	IContextMenu
	STDMETHOD(QueryContextMenu)(__in HMENU hmenu, __in UINT indexMenu, __in UINT idCmdFirst, __in UINT idCmdLast, __in  UINT uFlags);
	STDMETHOD(InvokeCommand)(__in CMINVOKECOMMANDINFO *pici);
	STDMETHOD(GetCommandString)(__in UINT_PTR idCmd, __in UINT uType, __reserved UINT *pReserved, __out_awcount(!(uType & GCS_UNICODE), cchMax) LPSTR pszName, __in UINT cchMax);	     

	//	IContextMenu2
	STDMETHOD(HandleMenuMsg)(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam);

	//	IContextMenu3
	STDMETHOD(HandleMenuMsg2)(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam, __out_opt LRESULT *plResult);
};

OBJECT_ENTRY_AUTO(__uuidof(CoPE32Viewer), CCoPE32Viewer)

