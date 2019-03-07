// PE32PropsView.h : Declaration of the CPE32PropsView
#pragma once
#pragma warning( disable : 4995 )

#include "resource.h"       // main symbols
#include <atlhost.h>
#include <uxtheme.h>

extern HINSTANCE hDLLInstance;

#include "PE32Viewer.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <strsafe.h>

class CCoPE32Viewer;
class CPE32Decoder;
class CPE32PropsView;

#include "IPE32ImportsCallbackImpl.h"
#include "IPE32ExportsCallbackImpl.h"
#include "IPE32CertificatesCallbackImpl.h"
#include "IPE32ResourcesCallbackImpl.h"
#include "IPE32StringsCallbackImpl.h"
#include "IPE32MessagesCallbackImpl.h"
#include "IPE32TypelibCallbackImpl.h"
#include "IPE32VersionCallbackImpl.h"

using namespace ATL;

#define VSCLASS_EXPLORER						L"EXPLORER"
#define VSOKBUTTON								L"OkButton"

typedef struct __tagRESOURCES_ENUM_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hTree;
}
RESOURCES_ENUM_CONTEXT, *PRESOURCES_ENUM_CONTEXT;

typedef bool (CALLBACK *PFN_ENUM_RES_BY_TYPE)(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);

typedef struct __tagRES_TYPE_ENUM_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hTree;
	HTREEITEM hti;
	PFN_ENUM_RES_BY_TYPE pfnEnumResByType;
}
RES_TYPE_ENUM_CONTEXT, *PRES_TYPE_ENUM_CONTEXT;

typedef struct __tagRES_TYPE_ENUM_FN_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hTree;
	HTREEITEM hti;
	DWORD dwIndex;
}
RES_TYPE_ENUM_FN_CONTEXT, *PRES_TYPE_ENUM_FN_CONTEXT;

typedef struct __tagIMPORTMODULE
{
	HTREEITEM hTreeItem;
	__wchar_t wsModuleName[MAX_PATH + 1];
}
IMPORTMODULE, *PIMPORTMODULE;

typedef bool (CALLBACK *PFN_ENUM_RES_TYPELIB)(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);

typedef struct __tagRES_TYPE_ENUM_TYPELIB_CONTEXT
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hDlg;
	HWND hTree;
	BSTR bstrTypeLibResName;
	PFN_ENUM_RES_TYPELIB pfnEnumTypeLib;
}
RES_TYPE_ENUM_TYPELIB_CONTEXT, *PRES_TYPE_ENUM_TYPELIB_CONTEXT;

typedef struct __tagRES_TYPE_ENUM_VERSION_STRINGS
{
	DWORD dwStructSize;
	CPE32PropsView* pMainDlg;
	HWND hTree;
	HTREEITEM hTreeRoot;
}
RES_TYPE_ENUM_VERSION_STRINGS, *PRES_TYPE_ENUM_VERSION_STRINGS;

typedef enum __tagETAB_ID
{
	tiHeaders,
	tiSections,
	tiImport,
	tiExport,
	tiSecurity,
	tiResources,
	tiTypeLib,
	tiManifest,
	tiRegistry,
	tiSystem,
	tiAbout
}
TAB_ID;

typedef struct __tagRES_TYPELIB_ENUM_CONTEXT
{
	DWORD dwStructSize;
	HWND hDlg;
	HWND hTab;
	RECT rcDisplay;
	DWORD dwIndex;
}
RES_TYPELIB_ENUM_CONTEXT, *PRES_TYPELIB_ENUM_CONTEXT;

typedef struct __tagRES_REGISTRY_UNIT
{
	DWORD dwStructSize;
	__PEIMGRESOURCEITEM Item;
}
RES_REGISTRY_UNIT, *PRES_REGISTRY_UNIT;

typedef struct __tagSYS_INFO_CTX
{
	DWORD dwStructSize;
	HWND hDlg;
	HWND hTree;
	HTREEITEM htiCPUBase;
	CPE32PropsView* pMainDlg;
	HANDLE hEvent;
	void* CPUVector;
	void* BBVector;
	void* OSVector;
}
SYS_INFO_CTX, *PSYS_INFO_CTX;

#define TYPELIB_VALID_HEADER_FLAG					0x5446534D
#define TYPELIB_VALID_HEADER_FLAG2					0x54474C53
#define TYPELIB_VERSION								0x00010002

typedef struct __tagTYPELIB_HDR
{
	DWORD dwValidHeaderFlag;
	DWORD dwVersion;
	DWORD dwGUIDOffset;
	DWORD dwLCID;
	DWORD dwUnknown;
	DWORD dwVarFlags;
	DWORD dwTypeLibVersion;
	DWORD dwFlags;
	DWORD dwTypeInfoCount;
	DWORD dwHelpStringOffset;
	DWORD dwHelpStringContext;
	DWORD dwHelpContext;
	DWORD dwNameTblCount;
	DWORD dwNameTblChars;
	DWORD dwTypelibNameOffset;
	DWORD dwHelpFileNameOffset;
	DWORD dwCustomDataOffset;
	DWORD dwReserved0x20;
	DWORD dwReserved0x80;
}
TYPELIB_HDR, *PTYPELIB_HDR;

typedef struct __tagDLG_TAB_ITEM
{
	DWORD dwStructSize;
	HWND hChildDlg;
	TAB_ID tiTabID;
	DWORD dwResID;
}
DLG_TAB_ITEM, *PDLG_TAB_ITEM;

#define PACKVERSION(major,minor) MAKELONG(minor,major)

// CPE32PropsView

typedef struct __tagUNK_RES_ITEM
{
	DWORD cbStructSize;
	DWORD dwResType;
	__PEIMGRESOURCEITEM Desc;
}
UNK_RES_ITEM, *PUNK_RES_ITEM;

class CPE32PropsView : 
	public CAxDialogImpl<CPE32PropsView>,
	public CIPE32ImportsCallbackImpl,
	public CIPE32ExportsCallbackImpl,
	public CIPE32CertificatesCallbackImpl,
	public CIPE32ResourcesCallbackImpl,
	public CIPE32StringsCallbackImpl,
	public CIPE32MessagesCallbackImpl,
	public CIPE32TypelibCallbackImpl,
	public CIPE32VersionCallbackImpl
{
public:
	CPE32Decoder* mf_pPE32;
	RECT mf_rcDisplay;
	CDSA<IMPORTMODULE> mf_ImportModules;
	CDSA<RES_REGISTRY_UNIT> mf_RegistryUnits;
	CDSA<UNK_RES_ITEM> mf_UnkResItem;
	CDPA mf_TypeLibRes;
	HWND mf_hwndChildTypeLib;
	HWND mf_hwndChildRegistry;
	RECT mf_rcTLDisplayRect;
	RECT mf_rcRegDisplayRect;
private:
	CPE32PropsView(void)															{}
	CPE32PropsView(const CPE32PropsView&)											{}
	CPE32PropsView(CPE32PropsView&&)												{}
	CPE32PropsView& operator= (const CPE32PropsView&)								{}
	CPE32PropsView& operator= (CPE32PropsView&&)									{}

	HICON mf_hDlgSmIcon;
	HICON mf_hDlgIcon;
	HCURSOR mf_hOuterCursor;
	HCURSOR mf_hInnerCursor;
	BOOL fbDWMExtendFrame;
	HTHEME mf_hTheme;
	POINT mf_ptInvokePt;
	HWND mf_hParent;
	HWND mf_hChildDlg;
	CDSA<DLG_TAB_ITEM> mf_DlgDescriptors;

	ULONG mf_nImportCount;
protected:
	static DLGTEMPLATE* LockDlgRes(__in __wchar_t* lpwszResName);
	static INT_PTR CALLBACK ChildHeaderDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildSectionsDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildImportDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildExportDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildSecurityDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildResourcesDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildTypeLibDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildTypeLibViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildManifestViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildRegistryDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildRegistryViewDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildSystemDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildAboutDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildViewAsAnsiDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildViewAsUnicodeDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildViewAsBlobDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildViewAsBitmapDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChildViewAsIconDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static HTREEITEM AddItemToTreeView(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in CComBSTR& lpwszInfoStr, __in LPARAM lParam = 0);
	static HTREEITEM AddBoldItemToTreeViewW(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in __wchar_t* lpwszInfoStr, __in const LPARAM lParam);
	static HTREEITEM AddItemToTreeViewA(__in HWND hTreeViewCtrl, __in HTREEITEM hParent, __in HTREEITEM hInsertAfter, __in char* lptszInfoStr);
	static void MakeReportView(__in HWND hListView, __in LPCWSTR lpwTitles[], __in int iTitlesCount);
	static void AddStringToReportView(__in HWND hListView, __in LPCWSTR lpwszStr[], int nColumnsCount);
	static WORD DecodeResourceID(__in DWORD dwResID);
	HWND GetChildDlgHandle(__in int nIndex, __out_opt BOOL* pfbCreated);
	static void FormatResource(__out CComBSTR& bstrResult, __in DWORD dwMsgID, ...);
	static PIMPORTMODULE FindImportModuleStruct(__in HDSA hDSA, __in __wchar_t* lpcwszModuleImgName);
	static PIMPORTMODULE AddImportModuleStruct(__in HDSA hDSA, __in HTREEITEM hTreeItem,__in __wchar_t* lpcwszModuleImgName);
	static void DisplayNode(__in IXMLDOMNode* pNode, __in HWND hTree, __in HTREEITEM hti);
	static void DumpFunc(__inout __wchar_t* pwstrNode, __in ITypeInfo* pti, __in FUNCDESC* pfuncdesc, __in TYPEATTR* pattr, __in MEMBERID memid);
	static void TYPEDESCtoString(__in ITypeInfo* pti, __in TYPEDESC* ptdesc, __inout BSTR* pbstr);
	static void GetTypeDesc(__in VARTYPE vt, __inout BSTR* pbstrDesc);
	static void CALLBACK GatherSysInfoCallback(__inout PTP_CALLBACK_INSTANCE Instance, __inout_opt  PVOID Context);
	static bool CALLBACK EnumIconTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumBitmapTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumGroupIconTypeResources(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumResourcesByType(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumPEImgVersionInfo(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumManifestResourcesByType(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);
	static bool CALLBACK EnumRegistryRes(__in DWORD dwIndex, __in DWORD dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in __in byte* pContext);

	HWND CreateChildTypeLibDlg(__in HWND hDlg, __in HWND hTab, __in UINT nDlgID);
	HWND CreateChildRegistryDlg(__in HWND hDlg, __in HWND hTab, __in UINT nDlgID);

	HWND CreateChildDialog(__in UINT nDlgID)
	{
		switch (nDlgID)
		{
		case IDD_DLGHEADER:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLGHEADER)), m_hWnd, ChildHeaderDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_SECTIONS:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_SECTIONS)), m_hWnd, ChildSectionsDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_IMPORT:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_IMPORT)), m_hWnd, ChildImportDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_EXPORT:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_EXPORT)), m_hWnd, ChildExportDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_SECURITY:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_SECURITY)), m_hWnd, ChildSecurityDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_RESOURCES:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_RESOURCES)), m_hWnd, ChildResourcesDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_TYPELIB:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_TYPELIB)), m_hWnd, ChildTypeLibDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_MANIFEST:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_MANIFEST)), m_hWnd, ChildManifestViewDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLG_REGISTRY:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLG_REGISTRY)), m_hWnd, ChildRegistryDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLGSYSTEM:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLGSYSTEM)), m_hWnd, ChildSystemDlgInitProc, reinterpret_cast<LPARAM>(this));
		case IDD_DLGABOUT:
			return ::CreateDialogIndirectParamW(hDLLInstance, LockDlgRes(MAKEINTRESOURCEW(IDD_DLGABOUT)), m_hWnd, ChildAboutDlgInitProc, reinterpret_cast<LPARAM>(this));
		default:
			return nullptr;
		}
	}
public:
	explicit CPE32PropsView(__in CPE32Decoder* pPE32Decoder, __in POINT ptInvoke, __in HWND hParent) :
		mf_hwndChildTypeLib(nullptr),
		mf_hwndChildRegistry(nullptr),
		mf_TypeLibRes(),
		mf_nImportCount(0),
		mf_ImportModules(),
		mf_RegistryUnits(),
	    mf_pPE32(pPE32Decoder),
		mf_ptInvokePt(ptInvoke),
		mf_hParent(hParent),
		mf_hChildDlg(nullptr),
		mf_DlgDescriptors(),
		mf_hOuterCursor(nullptr),
		mf_hInnerCursor(nullptr)
	{	}

	~CPE32PropsView()
	{	}

	static DWORD GetCommCtl32DllVersion(void);


	//	IPE32ImportsCallback
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
	STDMETHOD(EnumPE32ImportSymbols)(__in BSTR pwszModuleName, __in __PEIMGIMPORTITEM* pItemDesc, __in byte* pContext);

	//	IPE32ExportsCallback
	STDMETHOD(EnumPE32ExportSymbols)(__in unsigned long nIndex, __in __PEIMGEXPORTITEM* pItemDesc, __in byte* pContext);
	//	IPE32CertificatesCallback
	STDMETHOD(EnumPE32Certificates)(__in unsigned long dwIndex, __in BSTR pwszCertSubject, __in byte* pContext);
	//	IPE32ResourcesCallback
	STDMETHOD(EnumPE32ResourcesTypes)(__in unsigned long dwResType, __in BSTR bstrResTypeName, __in byte* pContext);
	STDMETHOD(EnumPE32ResourcesByType)(__in unsigned long dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in byte* pContext);
	//	IPE32StringsCallback
	STDMETHOD(EnumPE32StringResources)(__in LCID lcid, __in DWORD dwID, __in byte* pResData, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext);
	//	IPE32MessagesCallback
	STDMETHOD(EnumPE32MsgTableItemsResources)(__in LCID lcid, __in DWORD dwMsgID, __in byte* pResData, __in DWORD dwID, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext);
	//	IPE32TypelibCallback
	STDMETHOD(EnumPE32Typelib)(__in __PEIMGRESTYPELIBITEM* pTypelibDesc, __in byte* pContext);
	//	IPE32VersionCallback
	STDMETHOD(EnumPE32VersionStringTableStrings)(__in unsigned short wLanguage, __in unsigned short wCodePage, __in BSTR bstrKey, __in BSTR bstrValue, __in byte* pContext);

	enum { IDD = IDD_PE32PROPSVIEW };

BEGIN_MSG_MAP(CPE32PropsView)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroyDialog)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
	MESSAGE_HANDLER(WM_ACTIVATE, OnActivateDialog)
	MESSAGE_HANDLER(WM_CLOSE, OnCloseDialog)
	MESSAGE_HANDLER(WM_ENDSESSION, OnCloseDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	CHAIN_MSG_MAP(CAxDialogImpl<CPE32PropsView>)
	NOTIFY_HANDLER(IDC_TAB, TCN_SELCHANGE, OnTcnSelChangeTab)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnActivateDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!mf_hOuterCursor)
			mf_hOuterCursor = reinterpret_cast<HCURSOR>(::LoadImageW(nullptr, IDC_HAND, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTSIZE | LR_SHARED));
		::SetCursor(mf_hOuterCursor);
		bHandled = TRUE;
		return 0;
	}

	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		WORD xPos(GET_X_LPARAM(lParam)); 
		WORD yPos(GET_Y_LPARAM(lParam));

		POINT pt;
		pt.x = static_cast<LONG>(GET_X_LPARAM(lParam));
		pt.y = static_cast<LONG>(GET_Y_LPARAM(lParam));

		::ClientToScreen(m_hWnd, &pt);
		if (::WindowFromPhysicalPoint(pt) == m_hWnd)
		{
			if (!mf_hInnerCursor)
				mf_hInnerCursor = reinterpret_cast<HCURSOR>(::LoadImageW(nullptr, IDC_ARROW, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTSIZE | LR_SHARED));

			::SetCursor(mf_hInnerCursor);
		}
		bHandled = TRUE;
		return 0;
	}

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnDestroyDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (mf_hDlgSmIcon != ::LoadIcon(nullptr, IDI_ASTERISK))
			::DestroyIcon(mf_hDlgSmIcon);
		if (mf_hDlgIcon != ::LoadIcon(nullptr, IDI_ASTERISK))
			::DestroyIcon(mf_hDlgIcon);

		::DestroyCursor(mf_hOuterCursor);
		::DestroyCursor(mf_hInnerCursor);

		bHandled = FALSE;
		return 1;
	}

	LRESULT CloseDialog(int nRet)
    {
        if (m_bModal)
            EndDialog(nRet);
        else
        {
            DestroyWindow();
            ::PostQuitMessage(nRet);
        }
        return nRet;
    }

	LRESULT OnCloseDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//	Destroy child dialogs ...
		if (mf_DlgDescriptors.mf_HDSA)
		{
			int nCount(DSA_GetItemCount(mf_DlgDescriptors.mf_HDSA));
			if (nCount)
			{
				PDLG_TAB_ITEM pDTI;
				for (int nIndex(0); nIndex < nCount; ++nIndex)
				{
					if (nullptr != (pDTI = reinterpret_cast<PDLG_TAB_ITEM>(::DSA_GetItemPtr(mf_DlgDescriptors.mf_HDSA, nIndex))) && pDTI ->hChildDlg)
					{
						::DestroyWindow(pDTI ->hChildDlg);
						pDTI ->hChildDlg = nullptr;
					}
				}
			}
		}

		if (mf_hChildDlg) 
			mf_hChildDlg = nullptr;

		CloseDialog(0);
		bHandled = TRUE;
		return 0;
	};

	void WINAPI OnSelChanged(__in HWND hDlg) 
	{
		int iSel(TabCtrl_GetCurSel(GetDlgItem(IDC_TAB)));
		// Destroy the current child dialog box, if any. 
		if (mf_hChildDlg) 
		{
			::SetWindowLongW(mf_hChildDlg, GWL_STYLE, ::GetWindowLongW(mf_hChildDlg, GWL_STYLE) & ~WS_VISIBLE);
			::SetWindowPos(mf_hChildDlg, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			mf_hChildDlg = nullptr;
		}
 
		// Create the new child dialog box.
		BOOL fbCreated(FALSE);
		mf_hChildDlg = GetChildDlgHandle(iSel, &fbCreated);
		if (mf_hChildDlg)
		{
			::SetWindowLongW(mf_hChildDlg, GWL_STYLE, ::GetWindowLongW(mf_hChildDlg, GWL_STYLE) | WS_VISIBLE);
			::SetWindowPos(mf_hChildDlg, nullptr, 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}

	LRESULT OnTcnSelChangeTab(int nCtrlID, LPNMHDR pnmh, BOOL& bHandled)
	{
		OnSelChanged(::GetParent(pnmh ->hwndFrom));
		bHandled = TRUE;
		return TRUE;
	}
};
