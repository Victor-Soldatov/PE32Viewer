#pragma once
#include <Windows.h>
#include <atlbase.h>
#include "PE32Viewer_i.h"

//	Severity codes
#define SEVERITY_PE32VIEW_SUCCESS							0x00000000UL
#define SEVERITY_PE32VIEW_ERROR								0x00000001UL

//	Facility code
#define FACILITY_PE32VIEW									890UL

//	CPE32Decoder result codes
#define ERROR_PE32VIEW_SUCCESS								0UL
#define ERROR_PE32VIEW_UNINITIALIZED						1UL
#define ERROR_PE32VIEW_INITIALIZATION_FAILED				2UL
#define ERROR_PE32VIEW_UNEXPECTED_FAULT						3UL
#define ERROR_PE32VIEW_INVALID_ARGUMENTS					4UL
#define ERROR_PE32VIEW_INVALID_DOS_HEADER					5UL
#define ERROR_PE32VIEW_INVALID_NT_HEADER					6UL
#define ERROR_PE32VIEW_INVALID_OPT_HEADER					7UL
#define ERROR_PE32VIEW_WINTRUST_FAILED						8UL
#define ERROR_PE32VIEW_IMALLOC_FAILED						9UL
#define ERROR_PE32VIEW_NO_CERT_DESC							10UL
#define ERROR_PE32VIEW_NO_CERT_SUBJ							11UL
#define ERROR_PE32VIEW_NO_IMPORT							12UL
#define ERROR_PE32VIEW_CALLBACK_FAILED						13UL
#define ERROR_PE32VIEW_BREAKED_BY_CALLBACK					14UL
#define ERROR_PE32VIEW_NO_DELAY_IMPORT						15UL
#define ERROR_PE32VIEW_NO_CERTIFICATES						16UL
#define ERROR_PE32VIEW_NO_DOTNET_DESC						17UL
#define ERROR_PE32VIEW_DOTNET_DESC_UNKNOWN					18UL
#define ERROR_PE32VIEW_NO_EXPORT							19UL
#define ERROR_PE32VIEW_NO_RESOURCES							20UL
#define ERROR_PE32VIEW_NO_VERSION							21UL
#define ERROR_PE32VIEW_INVALID_VERSION_INFO					22UL
#define ERROR_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO		23UL
#define ERROR_PE32VIEW_NO_TYPELIB							24UL

//	CPE32Decoder HRESULT values
#define E_PE32VIEW_SUCCESS									MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_SUCCESS)
#define E_PE32VIEW_UNINITIALIZED							MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_UNINITIALIZED)
#define E_PE32VIEW_INITIALIZATION_FAILED					MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INITIALIZATION_FAILED)
#define E_PE32VIEW_UNEXPECTED_FAULT							MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_UNEXPECTED_FAULT)
#define E_PE32VIEW_INVALID_ARGUMENTS						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_ARGUMENTS)
#define E_PE32VIEW_INVALID_DOS_HEADER						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_DOS_HEADER)
#define E_PE32VIEW_INVALID_NT_HEADER						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_NT_HEADER)
#define E_PE32VIEW_INVALID_OPT_HEADER						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_OPT_HEADER)
#define E_PE32VIEW_WINTRUST_FAILED							MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_WINTRUST_FAILED)
#define E_PE32VIEW_IMALLOC_FAILED							MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_IMALLOC_FAILED)
#define E_PE32VIEW_NO_CERT_DESC								MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_CERT_DESC)
#define E_PE32VIEW_NO_CERT_SUBJ								MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_CERT_SUBJ)
#define E_PE32VIEW_NO_IMPORT								MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_IMPORT)
#define E_PE32VIEW_CALLBACK_FAILED							MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_CALLBACK_FAILED)
#define E_PE32VIEW_BREAKED_BY_CALLBACK						MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_BREAKED_BY_CALLBACK)
#define E_PE32VIEW_NO_DELAY_IMPORT							MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_DELAY_IMPORT)
#define E_PE32VIEW_NO_CERTIFICATES							MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_CERTIFICATES)
#define E_PE32VIEW_NO_DOTNET_DESC							MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_DOTNET_DESC)
#define E_PE32VIEW_DOTNET_DESC_UNKNOWN						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_DOTNET_DESC_UNKNOWN)
#define E_PE32VIEW_NO_EXPORT								MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_EXPORT)
#define E_PE32VIEW_NO_RESOURCES								MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_RESOURCES)
#define E_PE32VIEW_NO_VERSION								MAKE_HRESULT(SEVERITY_PE32VIEW_SUCCESS, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_VERSION)
#define E_PE32VIEW_INVALID_VERSION_INFO						MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_VERSION_INFO)
#define E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO			MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO)
#define E_PE32VIEW_NO_TYPELIB								MAKE_HRESULT(SEVERITY_PE32VIEW_ERROR, FACILITY_PE32VIEW, ERROR_PE32VIEW_NO_TYPELIB)

//	Other stuff
#define IMGHLP_ENUM_CERT_BUFFER								128

//	Base resources' types
#define RESOURCE_ID_CURSOR          1
#define RESOURCE_ID_BITMAP          2
#define RESOURCE_ID_ICON            3
#define RESOURCE_ID_MENU            4
#define RESOURCE_ID_DIALOG          5
#define RESOURCE_ID_STRING          6
#define RESOURCE_ID_FONTDIR         7
#define RESOURCE_ID_FONT            8
#define RESOURCE_ID_ACCELERATOR     9
#define RESOURCE_ID_RCDATA          10
#define RESOURCE_ID_MESSAGETABLE    11

#define RESOURCE_ID_VERSION			16
#define RESOURCE_ID_DLGINCLUDE		17
#define RESOURCE_ID_PLUGPLAY		19
#define RESOURCE_ID_VXD				20
#define RESOURCE_ID_ANICURSOR		21
#define RESOURCE_ID_ANIICON			22
#define RESOURCE_ID_HTML			23
#define RESOURCE_ID_MANIFEST        24

//	Types and vars
typedef struct __tagDOTNETDESC
{
    WORD	MajorRuntimeVersion;
    WORD	MinorRuntimeVersion;
    DWORD	dwFlags;	
}
DOTNETDESC, *LPDOTNETDESC;

typedef struct __tagVS_VERSIONINFO { 
  WORD  wLength; 
  WORD  wValueLength; 
  WORD  wType; 
  __wchar_t szKey[15]; 
  WORD  Padding1[1]; 
}
VS_VERSIONINFO;

typedef struct __tagVER_INFO_HDR
{
  WORD        wLength; 
  WORD        wValueLength; 
  WORD        wType; 
  __wchar_t   szKey[1];
}
VER_INFO_HDR, *PVER_INFO_HDR;

typedef struct __tagVER_STRING_TABLE 
{ 
  WORD   wLength; 
  WORD   wValueLength; 
  WORD   wType; 
  __wchar_t  szKey[8]; 
  WORD   Padding[1]; 
}
VER_STRING_TABLE, *PVER_STRING_TABLE;

typedef struct __tagVER_STRING
{ 
  WORD   wLength; 
  WORD   wValueLength; 
  WORD   wType; 
  __wchar_t  szKey[1]; 
}
VER_STRING, *PVER_STRING;

typedef struct __tagVER_VAR 
{ 
  WORD  wLength; 
  WORD  wValueLength; 
  WORD  wType; 
  __wchar_t szKey[1]; 
}
VER_VAR, *PVER_VAR;

typedef enum __E_RES_TYPE_LIB_STATE
{
	rsUnchecked,
	rsNonTypeLib,
	rsTypeLib
}
E_RES_TYPE_LIB_STATE;

typedef struct __tagRES_TYPE_NAME
{
	DWORD dwStructSize;
	E_RES_TYPE_LIB_STATE State;
	BSTR pwszShortName;
	BSTR pwszFullName;
}
RES_TYPE_NAME, *PRES_TYPE_NAME;

//	Class description
class CPE32Decoder
{
private:
	PIMAGE_DOS_HEADER mf_lpImgDOSHdr;
	PIMAGE_NT_HEADERS mf_lpImgNTHdrs;
	PIMAGE_FILE_HEADER mf_lpImgFileHdr;
	PIMAGE_OPTIONAL_HEADER mf_lpImgOptHdr;
	PIMAGE_DATA_DIRECTORY mf_lpImgDataDir;
	PIMAGE_SECTION_HEADER mf_lpImgSectionHdr;
	PIMAGE_DATA_DIRECTORY mf_lpImgImportDataDir;
	PIMAGE_IMPORT_DESCRIPTOR mf_lpImgImpDesc;
	PIMAGE_DATA_DIRECTORY mf_lpImgExportDataDir;
	PIMAGE_EXPORT_DIRECTORY mf_lpImgExportDir;
	PIMAGE_DATA_DIRECTORY mf_lpImgResourceDataDir;
	PIMAGE_RESOURCE_DIRECTORY mf_lpImgResourceDir;
	PIMAGE_SECTION_HEADER mf_lpImgResSection;

	HMODULE mf_hModule;
	HANDLE mf_hFileMap;
	LPVOID mf_lpImageBase;
	DWORD mf_mf_dwSize;
	DWORD mf_dwAllocationProtect;
	DWORD mf_dwProtect;
	DWORD mf_dwState;
	DWORD mf_dwType;
	ATL::CComBSTR mf_ModuleName;
	HRESULT mf_hInitResult;

	static int CALLBACK DPACmpConst(__in_opt const void *p1, __in_opt const void *p2, __in LPARAM lParam);
protected:
	STDMETHOD(InitializeContent)(__in __wchar_t* lpwszFileName);
	void ClearContent(void);
	LPBYTE GetFilePointer(__in DWORD dwRVA);
	PIMAGE_SECTION_HEADER GetSection(__in DWORD dwRVA);
	STDMETHOD(AnsiStrToWideCharStr)(__in char* lptszString, __in __wchar_t* lpwBuffer, __in UINT dwBufferSize, __in UINT uiCodePage);
	STDMETHOD(WideCharStrToAnsiStr)(__in __wchar_t* lpwszString, __in char* lptBuffer, __in UINT dwBufferSize, __in UINT uiCodePage);
	STDMETHOD(GetCertificateDescription)(__in PCCERT_CONTEXT pCertCtx, __out_opt BSTR* ppStrSertDesc);
	STDMETHOD(GetCertificateSubject)(__in PCCERT_CONTEXT pCert, __out_opt BSTR* ppStrSertSubj);
	STDMETHOD(AllocResourceNameCopy)(__in __wchar_t* lpwResName, __in DWORD dwNameLen, __out BSTR* ppwszResNameCopy);
	STDMETHOD(WalkImageResDirectory)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in int& cnItemsCount, __in_opt void* pContext);
	STDMETHOD(EnumResByTypeTotal)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in int& cnItemsCount);
	STDMETHOD(EnumImageResStrings)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32StringsCallback* pIPE32StringsCallback, __in int& cnItemsCount, __in_opt void* pContext);
	STDMETHOD(EnumImageMsgTableItems)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32MessagesCallback* pIPE32MessagesCallback, __in int& cnItemsCount, __in_opt void* pContext);
	STDMETHOD(FindVersionInfo)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __out_opt LPDWORD lpdwVISize, __out_opt VS_VERSIONINFO** ppVI);

	STDMETHOD(NameIsInList)(__in __notnull HDSA hResNamesArray, __in __notnull __wchar_t* const pwszShortName);
	STDMETHOD(EnumResNamesByType)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __inout HDSA hResNamesArray);

	STDMETHOD(FindManifest)(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __out_opt LPDWORD pdwManifestSize,__out_opt void** ppManifest);

	STDMETHOD(EnumStringTableString)(__in PVER_STRING pVS, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext, __out_opt LPWORD lpwLength);
	STDMETHOD(EnumStringTableContent)(__in PVER_STRING_TABLE pST, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext, __out_opt LPWORD wLength);
	STDMETHOD(EnumStringFileInfo)(__in PVER_INFO_HDR pSFI, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext);
public:
	CPE32Decoder(void);
	explicit CPE32Decoder(__in __wchar_t* lpwszFileName);
	CPE32Decoder(const CPE32Decoder& Src);
	CPE32Decoder(CPE32Decoder&& Src);
	CPE32Decoder& operator= (const CPE32Decoder&Src);
	CPE32Decoder& operator= (CPE32Decoder&& Src);
	virtual ~CPE32Decoder(void);

	STDMETHOD(TryDecodePE32)(__in __wchar_t* lpwszFileName);
	STDMETHOD(Clear)(void);
	STDMETHOD(InitResult)(void) const												{	return mf_hInitResult;	}

	STDMETHOD(GetPE32FileName)(__out_opt BSTR* pwszFileNameBuffer);
	STDMETHOD(GetFullPE32FileName)(__out_opt BSTR* pwszFileNameBuffer);
	
	STDMETHOD(GetDOSHeaderPtr)(__out_opt PIMAGE_DOS_HEADER* lpDOSHdr) const;
	STDMETHOD(GetNTHeadersPtr)(__out_opt PIMAGE_NT_HEADERS* lpNTHdrs) const;
	STDMETHOD(GetFileHeaderPtr)(__out_opt PIMAGE_FILE_HEADER* lpFileHdr) const;
	STDMETHOD(GetOptionalHeaderPtr)(__out_opt PIMAGE_OPTIONAL_HEADER* lpOptHdr) const;
	STDMETHOD(GetDataDirPtr)(__out_opt PIMAGE_DATA_DIRECTORY* lpDataDir) const;
	STDMETHOD(GetSectionsPtr)(__out_opt PIMAGE_SECTION_HEADER* lpSectHdr) const;

	STDMETHOD(IsPEImgFileDigitallySignedEx)(__out_opt HRESULT* pvWinTrustHResult);
	STDMETHOD(VerifyPEImageSignature)(__out_opt LPSYSTEMTIME lpSigDate, __out_opt LPSYSTEMTIME lpTimeStampDate, __out_opt BSTR* lppFileSigner, __out_opt BSTR* lppTimestampSigner);
	STDMETHOD(VerifyPEImageSignatureUI)(__in HWND hWnd, __out_opt LPSYSTEMTIME lpSigDate, __out_opt LPSYSTEMTIME lpTimeStampDate, __out_opt __wchar_t** lppFileSigner, __out_opt __wchar_t** lppTimestampSigner);
	STDMETHOD(IsPEImgFileDigitallySigned)(void);

	STDMETHOD(IsImport)(void);
	STDMETHOD(EnumPEImgImports)(__in IPE32ImportsCallback* pIPE32ImportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal);
	STDMETHOD(EnumPEImgDelayImports)(__in IPE32ImportsCallback* pIPE32ImportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal);
	STDMETHOD(EnumPEImgCertificates)(__in IPE32CertificatesCallback* pIPE32CertificatesCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal);
	STDMETHOD(IsPEImgDotNETDescriptorPresent)(__out_opt LPDOTNETDESC lpDotNETDesc);
	STDMETHOD(EnumPEImgExports)(__in IPE32ExportsCallback* pIPE32ExportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal);
	STDMETHOD(EnumPEImgResourceTypes)(__in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in_opt void* pContext);
	STDMETHOD(EnumPEImgResourcesByType)(__in DWORD dwResType, __in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in_opt void* pContext);
	STDMETHOD(EnumPEImgResourceStrings)(__in IPE32StringsCallback* pIPE32StringsCallback, __in_opt void* pContext);
	STDMETHOD(EnumPEImgMessageTableItems)(__in IPE32MessagesCallback* pIPE32MessagesCallback, __in_opt void* pContext);
	STDMETHOD(EnumTypeLibraries)(__in IPE32TypelibCallback* pIPE32TypelibCallback, __in_opt void* pContext);
	STDMETHOD(IsPEImgContainsVersion)(void);
	STDMETHOD(IsVersionInfoValid)(__in __notnull VS_VERSIONINFO* pVI);
	STDMETHOD(GetPEImgVersionFixedFileInfo)(__in __notnull VS_VERSIONINFO* pVI, __out_opt VS_FIXEDFILEINFO** ppFFI, __out WORD* pwFFILen);
	STDMETHOD(GetPEImgVersionTranslations)(__in __notnull VS_VERSIONINFO* pVI, __out_opt LANGANDCODEPAGE** ppvTranslationsPairs, __out_opt WORD* pwnLCPLen);
	STDMETHOD(EnumPEImgVersionStringTableStrings)(__in __notnull VS_VERSIONINFO* pVI, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext);
	STDMETHOD(IsPEImgContainsTypeLib)(void);
	STDMETHOD(IsPEImgContainsTypeLibEx)(__out_opt unsigned long *pulTypeLibResTotal);
	STDMETHOD(IsPEImgContainsManifest)(void);
	STDMETHOD(GetManifestContent)(__out_opt LPDWORD pdwManifestDataSize, __out_opt void** ppManifestContent);

	STDMETHOD(IsPEImgContainsRegistryRes)(void);
	STDMETHOD(GetPEImgRegistryResTypeID)(__out_opt LPDWORD lpdwRegistryResType);

	HANDLE GetSectionHandle(void) const												{	return mf_hFileMap;	}
};

