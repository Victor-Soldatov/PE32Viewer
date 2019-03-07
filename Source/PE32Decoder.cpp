#pragma warning( disable : 4995 )

#include "StdAfx.h"
#include <atlbase.h>
#include "PE32Viewer_i.h"

extern HINSTANCE hDLLInstance;

#include "PE32Decoder.h"
#include "PE32Misc.h"

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <Softpub.h>
#include <Wincrypt.h>
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
#include <mscat.h>
#include <Imagehlp.h>
#pragma comment(lib, "Imagehlp.lib")

#include <delayimp.h>
#include <shlwapi.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#define STRSAFE_NO_DEPRECATE 
#include <strsafe.h>
#pragma comment(lib, "strsafe.lib")
#include <memory>

using namespace ATL;

typedef LANGANDCODEPAGE *PLANGANDCODEPAGE;

const __wchar_t VerFileInfoKey[] = L"VS_VERSION_INFO";
const __wchar_t StringFileInfoKey[] = L"StringFileInfo";
const __wchar_t VarFileInfoKey[] = L"VarFileInfo";
const __wchar_t Translation[] = L"Translation";

#define VER_FILE_INFO_KEY_SIZE						ARRAYSIZE(VerFileInfoKey)
#define STRING_FILE_INFO_KEY_SIZE					ARRAYSIZE(StringFileInfoKey)
#define VAR_FILE_INFO_KEY_SIZE						ARRAYSIZE(VarFileInfoKey)
#define TRANSLATION_KEY_SIZE						ARRAYSIZE(Translation)
#define STRING_TABLE_KEY_SIZE						8

#define VS_FIXEDFILEINFO_SIGNATURE					0xFEEF04BD

//	Application constants ...

#define FILE_NAME_BUF_LEN							1024

extern "C++"
template <typename T>
__inline T* Inc(__in T* Ptr, __in_opt size_t nIncValue = 1UL)
{
	return reinterpret_cast<T*>(reinterpret_cast<UINT_PTR>(Ptr) + nIncValue);
}

#define PTR_SIZE									sizeof(void*)
#define PTR_32BIT_UNALIGN_MASK						(PTR_SIZE - 1UL)
#define PTR_32BIT_ALIGN_MASK						(~PTR_32BIT_UNALIGN_MASK)

extern "C++"
template <typename T>
__inline T* Align32(__in T* Ptr)
{
	return reinterpret_cast<T*>((reinterpret_cast<UINT_PTR>(Ptr) + PTR_32BIT_UNALIGN_MASK) & PTR_32BIT_ALIGN_MASK);
}

const __wchar_t pwsRegistryResType[] = L"REGISTRY";
const __wchar_t pwsTypeLibResType[] = L"TYPELIB";

#define REGISTRY_RESTYPE_NAME_LEN					(ARRAYSIZE(pwsRegistryResType) - 1)
#define TYPELIB_RESTYPE_NAME_LEN					(ARRAYSIZE(pwsTypeLibResType) - 1)

CPE32Decoder::CPE32Decoder(void) :
	mf_lpImgDOSHdr(nullptr),
	mf_lpImgNTHdrs(nullptr),
	mf_lpImgFileHdr(nullptr),
	mf_lpImgOptHdr(nullptr),
	mf_lpImgDataDir(nullptr),
	mf_lpImgSectionHdr(nullptr),
	mf_lpImgImportDataDir(nullptr),
	mf_lpImgImpDesc(nullptr),
	mf_lpImgExportDataDir(nullptr),
	mf_lpImgExportDir(nullptr),
	mf_lpImgResourceDataDir(nullptr),
	mf_lpImgResourceDir(nullptr),
	mf_lpImgResSection(nullptr),
	mf_hModule(nullptr),
	mf_hFileMap(nullptr),
	mf_lpImageBase(nullptr),
	mf_mf_dwSize(0),
	mf_dwAllocationProtect(0),
	mf_dwProtect(0),
	mf_dwState(0),
	mf_dwType(0),
	mf_hInitResult(E_PE32VIEW_UNINITIALIZED),
	mf_ModuleName()
{
}

CPE32Decoder::CPE32Decoder(__in __wchar_t* lpwszFileName) :
	mf_lpImgDOSHdr(nullptr),
	mf_lpImgNTHdrs(nullptr),
	mf_lpImgFileHdr(nullptr),
	mf_lpImgOptHdr(nullptr),
	mf_lpImgDataDir(nullptr),
	mf_lpImgSectionHdr(nullptr),
	mf_lpImgImportDataDir(nullptr),
	mf_lpImgImpDesc(nullptr),
	mf_lpImgExportDataDir(nullptr),
	mf_lpImgExportDir(nullptr),
	mf_lpImgResourceDataDir(nullptr),
	mf_lpImgResourceDir(nullptr),
	mf_lpImgResSection(nullptr),
	mf_hModule(nullptr),
	mf_hFileMap(nullptr),
	mf_lpImageBase(nullptr),
	mf_mf_dwSize(0),
	mf_dwAllocationProtect(0),
	mf_dwProtect(0),
	mf_dwState(0),
	mf_dwType(0),
	mf_hInitResult(E_PE32VIEW_UNINITIALIZED),
	mf_ModuleName()
{
	InitializeContent(lpwszFileName);
}

CPE32Decoder::~CPE32Decoder(void)
{
	ClearContent();
}

//	Opens file and map it
STDMETHODIMP CPE32Decoder::InitializeContent(__in __wchar_t* lpwszFileName)
{
	HRESULT hResult;
	ClearContent();

	if (!lpwszFileName)
	{
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
		return hResult;
	}

	if (mf_ModuleName.m_str)
		mf_ModuleName.Empty();
	mf_ModuleName = lpwszFileName;
	if (!mf_ModuleName.m_str)
	{
		hResult = ERROR_PE32VIEW_UNEXPECTED_FAULT;
		return hResult;
	}

	CXFileHandle File(::CreateFileW(lpwszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0));
	if (File == INVALID_HANDLE_VALUE)
	{
		hResult = __HRESULT_FROM_WIN32(::GetLastError());
		return hResult;
	}

	CXHandle FileMap(::CreateFileMappingW(File, nullptr, PAGE_READONLY, 0, 0, nullptr));
	if (!FileMap)
	{
		DWORD dwErrorCode(::GetLastError());
		hResult = __HRESULT_FROM_WIN32(dwErrorCode);
		return hResult;
	}

	CXMapView FileBase(::MapViewOfFile(FileMap, FILE_MAP_READ, 0, 0, 0));
	if (!FileBase)
	{
		hResult = __HRESULT_FROM_WIN32(::GetLastError());
		return hResult;
	}

	mf_hModule = reinterpret_cast<HMODULE>(FileBase.Leave());

	MEMORY_BASIC_INFORMATION mbi = {0};
	if (sizeof(MEMORY_BASIC_INFORMATION) != ::VirtualQuery(mf_hModule, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		//	There is an error occured.
		DWORD dwErrorCode(::GetLastError());
		ClearContent();
		hResult = __HRESULT_FROM_WIN32(::GetLastError());
		return hResult;
	}

	mf_lpImageBase = mbi.AllocationBase;
	mf_mf_dwSize = mbi.RegionSize;
	mf_dwAllocationProtect = mbi.AllocationProtect;
	mf_dwProtect = mbi.Protect;
	mf_dwState = mbi.State;
	mf_dwType = mbi.Type;

	mf_lpImgDOSHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(mf_hModule);
	if (mf_lpImgDOSHdr ->e_magic != IMAGE_DOS_SIGNATURE)
	{
		hResult = E_PE32VIEW_INVALID_DOS_HEADER;
		ClearContent();
		return hResult;
	}

	//	DOS header is valid.
	mf_lpImgNTHdrs = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<UINT_PTR>(mf_lpImgDOSHdr) + mf_lpImgDOSHdr ->e_lfanew);
	if (mf_lpImgNTHdrs ->Signature != IMAGE_NT_SIGNATURE)
	{
		hResult = E_PE32VIEW_INVALID_NT_HEADER;
		ClearContent();
		return hResult;
	}

	mf_lpImgFileHdr = &mf_lpImgNTHdrs ->FileHeader;
	mf_lpImgOptHdr = &mf_lpImgNTHdrs ->OptionalHeader;

	if (mf_lpImgOptHdr ->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		hResult = E_PE32VIEW_INVALID_OPT_HEADER;
		ClearContent();
		return hResult;
	}

	mf_lpImgDataDir = &mf_lpImgOptHdr ->DataDirectory[0];
	mf_lpImgSectionHdr = IMAGE_FIRST_SECTION(mf_lpImgNTHdrs);
	mf_hFileMap = FileMap.Leave();
	mf_hInitResult = hResult = E_PE32VIEW_SUCCESS;

	return hResult;
}

STDMETHODIMP CPE32Decoder::GetPE32FileName(__out_opt BSTR* pwszFileNameBuffer)
{
	if (pwszFileNameBuffer)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			CComBSTR FileName = mf_ModuleName;
			if (FileName.Length())
				::PathStripPathW(FileName.m_str);
			*pwszFileNameBuffer = FileName.Detach();
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetFullPE32FileName(__out_opt BSTR* pwszFileNameBuffer)
{
	if (pwszFileNameBuffer)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			CComBSTR Tmp = mf_ModuleName;
			*pwszFileNameBuffer = Tmp.Detach();
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

void CPE32Decoder::ClearContent(void)
{
	if (mf_hModule)
	{
		::UnmapViewOfFile(mf_hModule);
		mf_hModule = nullptr;
	}

	if (mf_hFileMap)
	{
		::CloseHandle(mf_hFileMap);
		mf_hFileMap = nullptr;
	}

	mf_lpImgDOSHdr = nullptr;
	mf_lpImgNTHdrs = nullptr;
	mf_lpImgFileHdr = nullptr;
	mf_lpImgOptHdr = nullptr;
	mf_lpImgDataDir = nullptr;
	mf_lpImgSectionHdr = nullptr;
	mf_lpImgImportDataDir = nullptr;
	mf_lpImgImpDesc = nullptr;
	mf_lpImgExportDataDir = nullptr;
	mf_lpImgExportDir = nullptr;
	mf_lpImgResourceDataDir = nullptr;
	mf_lpImgResourceDir = nullptr;
	mf_lpImgResSection = nullptr;
	mf_lpImageBase = nullptr;
	mf_mf_dwSize = 0;
	mf_dwAllocationProtect = 0;
	mf_dwProtect = 0;
	mf_dwState = 0;
	mf_dwType = 0;
	mf_ModuleName.Empty();
	mf_hInitResult = E_PE32VIEW_UNINITIALIZED;
}

STDMETHODIMP CPE32Decoder::Clear(void)
{
	ClearContent();
	return E_PE32VIEW_SUCCESS;
}

STDMETHODIMP CPE32Decoder::TryDecodePE32(__in __wchar_t* lpwszFileName)
{
	return InitializeContent(lpwszFileName);
}

STDMETHODIMP CPE32Decoder::GetDOSHeaderPtr(__out_opt PIMAGE_DOS_HEADER* lpDOSHdr) const
{
	if (lpDOSHdr)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpDOSHdr = mf_lpImgDOSHdr;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetNTHeadersPtr(__out_opt PIMAGE_NT_HEADERS* lpNTHdrs) const
{
	if (lpNTHdrs)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpNTHdrs = mf_lpImgNTHdrs;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetFileHeaderPtr(__out_opt PIMAGE_FILE_HEADER* lpFileHdr) const
{
	if (lpFileHdr)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpFileHdr = mf_lpImgFileHdr;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetOptionalHeaderPtr(__out_opt PIMAGE_OPTIONAL_HEADER* lpOptHdr) const
{
	if (lpOptHdr)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpOptHdr = mf_lpImgOptHdr;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetDataDirPtr(__out_opt PIMAGE_DATA_DIRECTORY* lpDataDir) const
{
	if (lpDataDir)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpDataDir = mf_lpImgDataDir;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::GetSectionsPtr(__out_opt PIMAGE_SECTION_HEADER* lpSectHdr) const
{
	if (lpSectHdr)
	{
		if (SUCCEEDED(mf_hInitResult))
		{
			*lpSectHdr = mf_lpImgSectionHdr;
			return ERROR_PE32VIEW_SUCCESS;
		}
		else
			return mf_hInitResult;
	}
	else
		return E_PE32VIEW_INVALID_ARGUMENTS;
}

STDMETHODIMP CPE32Decoder::IsPEImgFileDigitallySignedEx(__out_opt HRESULT* pvWinTrustHResult)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_SUCCESS);
	CXFileHandle FileHandle(::CreateFileW(mf_ModuleName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr));
	if (!FileHandle)
		hResult = HRESULT_FROM_WIN32(::GetLastError());
	else
	{
		PVOID Context;
		DWORD dwHashSize(0);
		CATALOG_INFO InfoStruct = { 0 };
		WINTRUST_DATA WintrustStructure = { 0 };
		WINTRUST_CATALOG_INFO WintrustCatalogStructure = { 0 };
		WINTRUST_FILE_INFO WintrustFileStructure = { 0 };

		InfoStruct.cbStruct = sizeof(CATALOG_INFO);
		WintrustCatalogStructure.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
		WintrustFileStructure.cbStruct = sizeof(WINTRUST_FILE_INFO);

		if (!::CryptCATAdminAcquireContext(&Context, nullptr, 0))
			hResult = E_PE32VIEW_WINTRUST_FAILED;
		else
		{
			::CryptCATAdminCalcHashFromFileHandle(FileHandle, &dwHashSize, nullptr, 0);
			if (!dwHashSize)
			{
				//	0 - sized has means error!
				::CryptCATAdminReleaseContext(Context, 0);
				hResult = __HRESULT_FROM_WIN32(::GetLastError());
			}
			else
			{
				auto Deleter = [](BYTE* pb) { if (pb) ::CoTaskMemFree(pb); };
				std::unique_ptr<BYTE, decltype(Deleter)> Buffer(reinterpret_cast<BYTE*>(::CoTaskMemAlloc(sizeof(BYTE) * dwHashSize)), Deleter);

				if (!Buffer)
				{
					if (!::CryptCATAdminCalcHashFromFileHandle(FileHandle, &dwHashSize, Buffer.get(), 0))
					{
							::CryptCATAdminReleaseContext(Context, 0);
							hResult = __HRESULT_FROM_WIN32(::GetLastError());
					}
					else
					{
						auto Deleter1 = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
						std::unique_ptr<__wchar_t, decltype(Deleter1)> MemberTag(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) * (dwHashSize + 1))), Deleter1);

						if (!MemberTag)
						{
							for (unsigned int i = 0; i < dwHashSize; i++)
								::StringCchPrintfW(&MemberTag.get()[i * 2], 3, L"%02X", Buffer.get()[i]);

							PVOID CatalogContext(::CryptCATAdminEnumCatalogFromHash(Context, Buffer.get(), dwHashSize, 0, nullptr));

							if (CatalogContext)
							{
								if (!::CryptCATCatalogInfoFromContext(CatalogContext, &InfoStruct, 0))
								{
									::CryptCATAdminReleaseCatalogContext(Context, CatalogContext, 0); 
									CatalogContext = nullptr;
								}
							}

							if (!CatalogContext)
							{
								WintrustFileStructure.cbStruct = sizeof(WINTRUST_FILE_INFO);
								WintrustFileStructure.pcwszFilePath = mf_ModuleName;
								WintrustFileStructure.hFile = nullptr;
								WintrustFileStructure.pgKnownSubject = nullptr;
								WintrustStructure.cbStruct = sizeof(WINTRUST_DATA);
								WintrustStructure.dwUnionChoice = WTD_CHOICE_FILE;
								WintrustStructure.pFile = &WintrustFileStructure;
								WintrustStructure.dwUIChoice = WTD_UI_NONE;
								WintrustStructure.fdwRevocationChecks = WTD_REVOKE_NONE;
								WintrustStructure.dwStateAction = WTD_STATEACTION_IGNORE;
								WintrustStructure.dwProvFlags = WTD_SAFER_FLAG;
								WintrustStructure.hWVTStateData = nullptr;
								WintrustStructure.pwszURLReference = nullptr;
							}
							else
							{
								WintrustStructure.cbStruct = sizeof(WINTRUST_DATA);
								WintrustStructure.pPolicyCallbackData = 0;
								WintrustStructure.pSIPClientData = 0;
								WintrustStructure.dwUIChoice = WTD_UI_NONE;
								WintrustStructure.fdwRevocationChecks = WTD_REVOKE_NONE;
								WintrustStructure.dwUnionChoice = WTD_CHOICE_CATALOG;
								WintrustStructure.pCatalog = &WintrustCatalogStructure;
								WintrustStructure.dwStateAction = WTD_STATEACTION_VERIFY;
								WintrustStructure.hWVTStateData = nullptr;
								WintrustStructure.pwszURLReference = nullptr;
								WintrustStructure.dwProvFlags = 0;
								WintrustStructure.dwUIContext = WTD_UICONTEXT_EXECUTE;

								WintrustCatalogStructure.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
								WintrustCatalogStructure.dwCatalogVersion = 0;
								WintrustCatalogStructure.pcwszCatalogFilePath = InfoStruct.wszCatalogFile;
								WintrustCatalogStructure.pcwszMemberTag = MemberTag.get();
								WintrustCatalogStructure.pcwszMemberFilePath = mf_ModuleName;
								WintrustCatalogStructure.hMemberFile = nullptr;
							}

							GUID ActionGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2; 
							HRESULT hReturnVal(static_cast<HRESULT>(::WinVerifyTrust(0, &ActionGuid, &WintrustStructure)));

							if (pvWinTrustHResult)
								*pvWinTrustHResult = hReturnVal;

							if (CatalogContext) 
								::CryptCATAdminReleaseCatalogContext(Context, CatalogContext, 0);

							if (SUCCEEDED(hReturnVal))
							{
								WintrustStructure.dwStateAction = WTD_STATEACTION_CLOSE;
								:: WinVerifyTrust(0, &ActionGuid, &WintrustStructure);
							}
						}
						else
							hResult = E_PE32VIEW_IMALLOC_FAILED;
					}
				}
				else
					hResult = E_PE32VIEW_IMALLOC_FAILED;
			}
		}
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::GetCertificateDescription(__in PCCERT_CONTEXT pCertCtx, __out_opt BSTR* ppStrSertDesc)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	if (ppStrSertDesc)
	{
		DWORD dwStrType(CERT_X500_NAME_STR);
		DWORD dwCount(::CertGetNameStringW(pCertCtx, CERT_NAME_RDN_TYPE, 0, &dwStrType, nullptr, 0));
		if (dwCount)
		{
			auto Deleter = [](BSTR pStr) { if (pStr) ::SysFreeString(pStr); };
			std::unique_ptr<OLECHAR, decltype(Deleter)> szSubjectRDN(::SysAllocStringLen(nullptr, dwCount), Deleter);

			if (szSubjectRDN)
			{
				::CertGetNameStringW(pCertCtx, CERT_NAME_RDN_TYPE, 0, &dwStrType, szSubjectRDN.get(), dwCount);
				*ppStrSertDesc = szSubjectRDN.release();
			}
			else 
			{
				*ppStrSertDesc = nullptr;
				hResult = E_PE32VIEW_IMALLOC_FAILED;
			}
		}
		else 
		{
			*ppStrSertDesc = nullptr;
			hResult = ERROR_PE32VIEW_NO_CERT_DESC;
		}
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;

   return hResult;
}

STDMETHODIMP CPE32Decoder::GetCertificateSubject(__in PCCERT_CONTEXT pCertCtx, __out_opt BSTR* ppStrSertSubj)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (ppStrSertSubj)
	{
		DWORD dwStrType(CERT_X500_NAME_STR);
		DWORD dwCount(::CertGetNameStringW(pCertCtx, CERT_NAME_RDN_TYPE, 0, &dwStrType, nullptr, 0));
		if (dwCount)
		{
			auto Deleter = [](BSTR pStr) { if (pStr) ::SysFreeString(pStr); };
			std::unique_ptr<OLECHAR, decltype(Deleter)> lpwszSubjectRDN(::SysAllocStringLen(nullptr, dwCount), Deleter);

			if (lpwszSubjectRDN)
			{
				dwCount = ::CertGetNameStringW(pCertCtx, CERT_NAME_RDN_TYPE, 0, &dwStrType, lpwszSubjectRDN.get(), dwCount);
				if (dwCount)
					*ppStrSertSubj = lpwszSubjectRDN.release();
				else
					hResult = E_PE32VIEW_NO_CERT_SUBJ;
			}
			else
			{
				*ppStrSertSubj = nullptr;
				hResult = E_PE32VIEW_IMALLOC_FAILED;
			}
		}
		else
		{
			*ppStrSertSubj = nullptr;
			hResult = E_PE32VIEW_NO_CERT_SUBJ;
		}
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return (hResult);
}

STDMETHODIMP CPE32Decoder::VerifyPEImageSignature(__out_opt LPSYSTEMTIME lpSigDate, __out_opt LPSYSTEMTIME lpTimeStampDate, __out_opt BSTR* lppFileSigner, __out_opt BSTR* lppTimestampSigner)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_SUCCESS);

	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_FILE_INFO sWintrustFileInfo = { 0 };
	WINTRUST_DATA sWintrustData = { 0 };

	CXFileHandle FileHandle(::CreateFileW(mf_ModuleName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr));

    if (!FileHandle)
		hResult = HRESULT_FROM_WIN32(::GetLastError());
	else
	{

		sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
		sWintrustFileInfo.pcwszFilePath = mf_ModuleName;
		sWintrustFileInfo.hFile = FileHandle;

		sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
		sWintrustData.dwUIChoice          = WTD_UI_NONE;
		sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
		sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
		sWintrustData.pFile               = &sWintrustFileInfo;
		sWintrustData.dwStateAction       = WTD_STATEACTION_VERIFY;

		LONG lnWinTrustResult;
		if (0 == (lnWinTrustResult = ::WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData)))
		{
			//	File signature is OK.
			//	Try to retreive the signer certificate and display its information
			CRYPT_PROVIDER_DATA const *psProvData(nullptr);
			CRYPT_PROVIDER_SGNR *psProvSigner(nullptr);
			CRYPT_PROVIDER_CERT *psProvCert(nullptr);
			FILETIME localFt = { 0 };
			SYSTEMTIME sysTime = { 0 };

			psProvData = ::WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
			if (psProvData)
			{
				psProvSigner = ::WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0 , FALSE, 0);
				if (psProvSigner)
				{
					if (lpSigDate)
					{
						::FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
						::FileTimeToSystemTime(&localFt, &sysTime);
						::CopyMemory(lpSigDate, &sysTime, sizeof(SYSTEMTIME));
					}

					psProvCert = ::WTHelperGetProvCertFromChain(psProvSigner, 0);
					if (psProvCert)
					{
						CComBSTR szCertDesc;
						if (SUCCEEDED(hResult = GetCertificateDescription(psProvCert->pCert, &szCertDesc.m_str)) && lppFileSigner)
							*lppFileSigner = szCertDesc.Detach();
					}

					if (psProvSigner->csCounterSigners)
					{
						// Timestamp information
						if (lpTimeStampDate)
						{
							::FileTimeToLocalFileTime(&psProvSigner->pasCounterSigners[0].sftVerifyAsOf, &localFt);
							::FileTimeToSystemTime(&localFt, &sysTime);
							::CopyMemory(lpTimeStampDate, &sysTime, sizeof(SYSTEMTIME));
						}

						psProvCert = ::WTHelperGetProvCertFromChain(&psProvSigner->pasCounterSigners[0], 0);
						if (psProvCert)
						{
							CComBSTR szCertDesc;
							if (SUCCEEDED(hResult = GetCertificateDescription(psProvCert->pCert,  &szCertDesc.m_str)) && lppTimestampSigner)
								*lppTimestampSigner = szCertDesc.Detach();
						}
					}
				}
				else
					hResult = ERROR_PE32VIEW_WINTRUST_FAILED;
			}
			else
				hResult = ERROR_PE32VIEW_WINTRUST_FAILED;
		}
		else
			hResult = lnWinTrustResult;
   
		sWintrustData.dwUIChoice = WTD_UI_NONE;
		sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		::WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::VerifyPEImageSignatureUI(__in HWND hWnd, __out_opt LPSYSTEMTIME lpSigDate, __out_opt LPSYSTEMTIME lpTimeStampDate, __out_opt __wchar_t** lppFileSigner, __out_opt __wchar_t** lppTimestampSigner)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_SUCCESS);

	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_FILE_INFO sWintrustFileInfo = { 0 };
	WINTRUST_DATA sWintrustData = { 0 };

	CXFileHandle FileHandle(::CreateFileW(mf_ModuleName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr));
    if (!FileHandle)
		hResult = HRESULT_FROM_WIN32(::GetLastError());
	else
	{
		sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
		sWintrustFileInfo.pcwszFilePath = mf_ModuleName;
		sWintrustFileInfo.hFile = FileHandle;
	
		sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
		sWintrustData.dwUIChoice          = WTD_UI_ALL;
		sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
		sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
		sWintrustData.pFile               = &sWintrustFileInfo;
		sWintrustData.dwStateAction       = WTD_STATEACTION_VERIFY;

		LONG lnWinTrustResult;
		if (ERROR_SUCCESS == (lnWinTrustResult = ::WinVerifyTrust(hWnd, &guidAction, &sWintrustData)))
		{
			//	File signature is OK.
			//	Try to retreive the signer certificate and display its information
			CRYPT_PROVIDER_DATA const *psProvData(nullptr);
			CRYPT_PROVIDER_SGNR* psProvSigner(nullptr);
			CRYPT_PROVIDER_CERT* psProvCert(nullptr);
			FILETIME localFt = { 0 };
			SYSTEMTIME sysTime = { 0 };

			psProvData = ::WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
			if (psProvData)
			{
				psProvSigner = ::WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0 , FALSE, 0);
				if (psProvSigner)
				{
					if (lpSigDate)
					{
						::FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
						::FileTimeToSystemTime(&localFt, &sysTime);
						::CopyMemory(lpSigDate, &sysTime, sizeof(SYSTEMTIME));
					}

					psProvCert = ::WTHelperGetProvCertFromChain(psProvSigner, 0);
					if (psProvCert)
					{
						CComBSTR szCertDesc;
						if (SUCCEEDED(hResult = GetCertificateDescription(psProvCert->pCert, &szCertDesc.m_str)) && lppFileSigner)
						{
							*lppFileSigner = szCertDesc.Detach();
							szCertDesc.Empty();
						}
					}

					if (psProvSigner->csCounterSigners)
					{
						// Timestamp information
						if (lpTimeStampDate)
						{
							::FileTimeToLocalFileTime(&psProvSigner->pasCounterSigners[0].sftVerifyAsOf, &localFt);
							::FileTimeToSystemTime(&localFt, &sysTime);
							::CopyMemory(lpTimeStampDate, &sysTime, sizeof(SYSTEMTIME));
						}

						psProvCert = ::WTHelperGetProvCertFromChain(&psProvSigner->pasCounterSigners[0], 0);
						if (psProvCert)
						{
							CComBSTR szCertDesc;
							if (SUCCEEDED(hResult = GetCertificateDescription(psProvCert->pCert, &szCertDesc.m_str)) && lppTimestampSigner)
							{
								*lppTimestampSigner = szCertDesc.Detach();
								szCertDesc.Empty();
							}
						}
					}
				}
			}
			else
				hResult = E_PE32VIEW_WINTRUST_FAILED;
		}
		else
			hResult = lnWinTrustResult;
   
		sWintrustData.dwUIChoice = WTD_UI_NONE;
		sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
		::WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgFileDigitallySigned(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_SUCCESS);

	CXFileHandle FileHandle(::CreateFileW(mf_ModuleName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr));
    if (!FileHandle)
		hResult = HRESULT_FROM_WIN32(::GetLastError());
	else
	{
		PVOID Context;
		CATALOG_INFO InfoStruct = { 0 };
		WINTRUST_DATA WintrustStructure = { 0 };
		WINTRUST_CATALOG_INFO WintrustCatalogStructure = { 0 };
		WINTRUST_FILE_INFO WintrustFileStructure = { 0 };
		ULONG ReturnVal;
		GUID ActionGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2; 

		//	Zero structures.

		InfoStruct.cbStruct = sizeof(CATALOG_INFO);
		WintrustCatalogStructure.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
		WintrustFileStructure.cbStruct = sizeof(WINTRUST_FILE_INFO);

		//	Get a context for signature verification.

		if (!::CryptCATAdminAcquireContext(&Context, nullptr, 0))
			hResult = E_PE32VIEW_WINTRUST_FAILED;
		else
		{
			DWORD dwHashSize(0);
			::CryptCATAdminCalcHashFromFileHandle(FileHandle, &dwHashSize, nullptr, 0);
			if (!dwHashSize)
			{
				::CryptCATAdminReleaseContext(Context, 0);
				hResult = E_PE32VIEW_WINTRUST_FAILED;
			}
			else
			{
				//	Allocate memory.
				auto Deleter = [](PBYTE pb) { if (pb) ::CoTaskMemFree(pb); };
				std::unique_ptr<BYTE, decltype(Deleter)> Buffer(reinterpret_cast<PBYTE>(::CoTaskMemAlloc(dwHashSize)), Deleter);

				if (Buffer)
				{
					//	Actually calculate the hash
					if (!::CryptCATAdminCalcHashFromFileHandle(FileHandle, &dwHashSize, Buffer.get(), 0))
					{
							::CryptCATAdminReleaseContext(Context, 0);
							return E_FAIL;
					}
	   
					//	Convert the hash to a string.
					CComBSTR MemberTag = ::SysAllocStringLen(nullptr, dwHashSize * 2);
					if (MemberTag)
					{
						::ZeroMemory(MemberTag.m_str, dwHashSize * 2 * sizeof(__wchar_t));
						PVOID CatalogContext;

						for (unsigned int i(0); i < dwHashSize; ++i)
							::StringCchPrintfW(&MemberTag.m_str[i * 2], 3, L"%02X", Buffer.get()[i]);

						//	Get catalog for our context.
						CatalogContext = ::CryptCATAdminEnumCatalogFromHash(Context, Buffer.get(), dwHashSize, 0, nullptr);
						if (CatalogContext)
						{
								//If we couldn’t get information
							if (!::CryptCATCatalogInfoFromContext(CatalogContext, &InfoStruct, 0))
							{
								//Release the context and set the context to null so it gets picked up below.
								::CryptCATAdminReleaseCatalogContext(Context, CatalogContext, 0); 
								CatalogContext = nullptr;
							}
						}

						//	If we have a valid context, we got our info.
						//	Otherwise, we attempt to verify the internal signature.

						if (!CatalogContext)
						{
							WintrustFileStructure.cbStruct = sizeof(WINTRUST_FILE_INFO);
							WintrustFileStructure.pcwszFilePath = mf_ModuleName;
							WintrustFileStructure.hFile = nullptr;
							WintrustFileStructure.pgKnownSubject = nullptr;
							WintrustStructure.cbStruct = sizeof(WINTRUST_DATA);
							WintrustStructure.dwUnionChoice = WTD_CHOICE_FILE;
							WintrustStructure.pFile = &WintrustFileStructure;
							WintrustStructure.dwUIChoice = WTD_UI_NONE;
							WintrustStructure.fdwRevocationChecks = WTD_REVOKE_NONE;
							WintrustStructure.dwStateAction = WTD_STATEACTION_IGNORE;
							WintrustStructure.dwProvFlags = WTD_SAFER_FLAG;
							WintrustStructure.hWVTStateData = nullptr;
							WintrustStructure.pwszURLReference = nullptr;
						}
						else
						{
							//	If we get here, we have catalog info! Verify it.
							WintrustStructure.cbStruct = sizeof(WINTRUST_DATA);
							WintrustStructure.pPolicyCallbackData = 0;
							WintrustStructure.pSIPClientData = 0;
							WintrustStructure.dwUIChoice = WTD_UI_NONE;
							WintrustStructure.fdwRevocationChecks = WTD_REVOKE_NONE;
							WintrustStructure.dwUnionChoice = WTD_CHOICE_CATALOG;
							WintrustStructure.pCatalog = &WintrustCatalogStructure;
							WintrustStructure.dwStateAction = WTD_STATEACTION_VERIFY;
							WintrustStructure.hWVTStateData = nullptr;
							WintrustStructure.pwszURLReference = nullptr;
							WintrustStructure.dwProvFlags = 0;
							WintrustStructure.dwUIContext = WTD_UICONTEXT_EXECUTE;

							//	Fill in catalog info structure.
							WintrustCatalogStructure.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
							WintrustCatalogStructure.dwCatalogVersion = 0;
							WintrustCatalogStructure.pcwszCatalogFilePath = InfoStruct.wszCatalogFile;
							WintrustCatalogStructure.pcwszMemberTag = MemberTag;
							WintrustCatalogStructure.pcwszMemberFilePath = mf_ModuleName;
							WintrustCatalogStructure.hMemberFile = nullptr;
						}

						//	Call our verification function ...
						ReturnVal = ::WinVerifyTrust(0, &ActionGuid, &WintrustStructure); 

						//	Check return ...
						if (ERROR_SUCCESS != ReturnVal)
						{
							switch(ReturnVal)
							{
							case TRUST_E_SUBJECT_NOT_TRUSTED:
							case TRUST_E_PROVIDER_UNKNOWN:
							case TRUST_E_ACTION_UNKNOWN:
								hResult = S_FALSE;
								break;
							default:
								hResult = S_OK;
								break;
							}
						}
						else
							hResult = S_OK;

						//	Free context ...
						if (CatalogContext) 
							::CryptCATAdminReleaseCatalogContext(Context, CatalogContext, 0);

						//	If we successfully verified, we need to free ...
						if (0 == ReturnVal)
						{
							WintrustStructure.dwStateAction = WTD_STATEACTION_CLOSE;
							:: WinVerifyTrust(0, &ActionGuid, &WintrustStructure);
						} 
					}
				}
				::CryptCATAdminReleaseContext(Context, 0);
			}
		}
	}
	return hResult;
}

LPBYTE CPE32Decoder::GetFilePointer(__in DWORD dwRVA)
{
	PIMAGE_SECTION_HEADER pSectHeader(GetSection(dwRVA));
	if (!pSectHeader)
		return (reinterpret_cast<LPBYTE>(reinterpret_cast<UINT_PTR>(mf_lpImageBase) + static_cast<UINT_PTR>(dwRVA))); //	... RVA is file begin offset ...
	else
	//	... else calculate section begin offset ...
		return (reinterpret_cast<LPBYTE>(reinterpret_cast<UINT_PTR>(mf_lpImageBase) + pSectHeader->PointerToRawData + (dwRVA - pSectHeader->VirtualAddress)));
}

PIMAGE_SECTION_HEADER CPE32Decoder::GetSection(__in DWORD dwRVA)
{
	PIMAGE_SECTION_HEADER pSectHeader(IMAGE_FIRST_SECTION(mf_lpImgNTHdrs));
	for (WORD i(0); i < mf_lpImgFileHdr ->NumberOfSections; ++i, ++pSectHeader) 
	{
		//	If RVA is in, returns section's header's ptr
		if (dwRVA >= pSectHeader->VirtualAddress && dwRVA < (pSectHeader->VirtualAddress + pSectHeader->Misc.VirtualSize))
			return pSectHeader;
	}
	//	Section is not found
	return nullptr; 
}

STDMETHODIMP CPE32Decoder::AnsiStrToWideCharStr(__in char* lptszString, __in __wchar_t* lpwBuffer, __in UINT dwBufferSize, __in UINT uiCodePage)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	if (lptszString)
	{
		UINT dwcnChars(0);
		//	Try to get ansi string length (in chars)
		hResult = ::StringCchLengthA((STRSAFE_PCNZCH)(lptszString), STRSAFE_MAX_CCH, &dwcnChars);
		if (SUCCEEDED(hResult))
		{
			UINT dwcnWChars(::MultiByteToWideChar(uiCodePage, 0, lptszString, -1, nullptr, 0));
			if (dwcnWChars) 
			{				
				if (dwcnWChars < dwBufferSize)
				{
					::ZeroMemory(lpwBuffer, dwBufferSize * sizeof(__wchar_t));
					if (0 != ::MultiByteToWideChar(uiCodePage, 0, lptszString, -1, lpwBuffer, dwcnWChars))
						hResult = E_PE32VIEW_SUCCESS;
					else
						hResult = __HRESULT_FROM_WIN32(::GetLastError());
				}
				else
					hResult = __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			}
			else
				hResult = __HRESULT_FROM_WIN32(::GetLastError());
		}
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::WideCharStrToAnsiStr(__in __wchar_t* lpwszString, __in char* lptBuffer, __in UINT dwBufferSize, __in UINT uiCodePage)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	if (lpwszString)
	{
		CPINFOEXA cpiEx = {0};
		if (::GetCPInfoExA(uiCodePage, 0, &cpiEx))
		{
			LPCSTR lpDefaultChar(((uiCodePage == CP_UTF7) || (uiCodePage == CP_UTF8)) ? nullptr : ((LPCSTR)(&cpiEx.DefaultChar)));
			BOOL fbDefaultCharIsUsed(FALSE);
			LPBOOL lpDefaultCharIsUsedFlag(((uiCodePage == CP_UTF7) || (uiCodePage == CP_UTF8)) ? nullptr : &fbDefaultCharIsUsed);
			UINT dwcnWideChars(0);
			//	Try to get unicode string length (in wchars)
			hResult = ::StringCchLengthW((STRSAFE_PCNZWCH)(lpwszString), STRSAFE_MAX_CCH, &dwcnWideChars);
			if (SUCCEEDED(hResult))
			{
				UINT dwcnTChars(::WideCharToMultiByte(uiCodePage, 0, lpwszString, -1, nullptr, 0, lpDefaultChar, lpDefaultCharIsUsedFlag));
				if (dwcnTChars)
				{
					//	Perform translation
					::ZeroMemory(lptBuffer, dwBufferSize * sizeof(char));
					if (dwcnTChars < dwBufferSize)
					{
						if (0 != ::WideCharToMultiByte(uiCodePage, 0, lpwszString, -1, lptBuffer, dwcnTChars, lpDefaultChar, lpDefaultCharIsUsedFlag))
							hResult = E_PE32VIEW_SUCCESS;
						else
							hResult = __HRESULT_FROM_WIN32(::GetLastError());
					}
					else
						hResult = __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
				}
				else
					hResult = __HRESULT_FROM_WIN32(::GetLastError());
			}
		}
		else
			hResult = __HRESULT_FROM_WIN32(::GetLastError());
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::IsImport(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_SUCCESS);
	if (!mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		hResult = S_FALSE;

	return hResult;
}

int CALLBACK CPE32Decoder::DPACmpConst(__in_opt const void *p1, __in_opt const void *p2, __in LPARAM lParam)
{
	if (p1 != p2)
	{
		if (p1 < p2)
			return -1;
		else 
			return 1;
	}
	else
		return 0;
	UNREFERENCED_PARAMETER(lParam);
}

STDMETHODIMP CPE32Decoder::EnumPEImgImports(__in IPE32ImportsCallback* pIPE32ImportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32ImportsCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32ImportsCallback));

		int nImportsItemsCount(0);
		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		{
			PIMAGE_DATA_DIRECTORY lpImportDataDir(&mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT]);

			for (PIMAGE_IMPORT_DESCRIPTOR pImportDesc(reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(GetFilePointer(lpImportDataDir ->VirtualAddress))); pImportDesc->Name != 0; ++pImportDesc) 
			{
				__wchar_t lpwszModuleName[MAX_PATH + 1] = { 0 };
				char* lptszModuleName = reinterpret_cast<char*>(GetFilePointer(pImportDesc->Name));
				
				AnsiStrToWideCharStr(lptszModuleName, lpwszModuleName, MAX_PATH, CP_ACP);

				time_t time = pImportDesc->TimeDateStamp;

				bool fbBounded(false), fbOldBind(false);
				if (time == -1) 
					//	New style bounding.
					fbBounded = true;
				else 
					if (time)
						//	Old style bounding.
						fbBounded = fbOldBind = true;

				PIMAGE_THUNK_DATA32 pINT;
				PIMAGE_THUNK_DATA32 pIAT;
				if (pImportDesc->OriginalFirstThunk != 0) 
				{
					pINT = reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pImportDesc->OriginalFirstThunk));
					pIAT = reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pImportDesc->FirstThunk));
				} 
				else 
				{	//	Quirks of old good Borland's TLink ...
					pINT = reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pImportDesc->FirstThunk));
					pIAT = nullptr;
					fbBounded = false;
				}

				CDPA ForwardRefs;

				if (ForwardRefs.mf_hDPA && (pImportDesc->ForwarderChain != -1) && fbOldBind) 
					for (DWORD dwChain(pImportDesc->ForwarderChain); dwChain != -1;  dwChain = pIAT[dwChain].u1.Ordinal)
						DPA_SortedInsertPtr(ForwardRefs.mf_hDPA, reinterpret_cast<void*>(dwChain), 0, DPACmpConst, 0, DPAS_INSERTAFTER, reinterpret_cast<void*>(dwChain));

				for (DWORD i(0); pINT->u1.Ordinal != 0; ++i) 
				{
					__PEIMGIMPORTITEM ii = { 0 };
					ii.dwStructSize = sizeof(__PEIMGIMPORTITEM);
					ii.BoundState = fbBounded ? bsBounded : bsUnbounded;
					if (IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal))
					{
						ii.dwOrdinal = pINT->u1.Ordinal & ~IMAGE_ORDINAL_FLAG;
						ii.ImportKind = ikByOrdinal;
						ii.wHint = 0;
					}
					else 
					{
						PIMAGE_IMPORT_BY_NAME p(reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(GetFilePointer(pINT->u1.Ordinal)));
						ii.ImportKind = ikByName;
						ii.pAnsiName = reinterpret_cast<unsigned char*>(p ->Name);
						ii.wHint = p ->Hint;
					}

					if (fbOldBind && (-1 != ::DPA_Search(ForwardRefs.mf_hDPA, reinterpret_cast<void*>(i), 0, DPACmpConst, 0, DPAS_SORTED)))
						ii.ForwardState = fsForwarded;
					else
						ii.ForwardState = fsNotForwarded;

					++nImportsItemsCount;
					CComBSTR ModuleName = &lpwszModuleName[0];

					HRESULT hCBResult(pIPE32ImportsCallback ->EnumPE32ImportSymbols(ModuleName.Detach(), &ii, reinterpret_cast<byte*>(pContext)));
					if (FAILED(hCBResult))
					{
						hResult = E_PE32VIEW_CALLBACK_FAILED;
						break;
					}
					if (S_OK != hCBResult)
					{
						hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
						break;
					}

					++pINT;
					if (pIAT) 
						++pIAT;
				}
			}
		}
		else
			hResult = E_PE32VIEW_NO_IMPORT;

		if (lpnCallsTotal)
			*lpnCallsTotal = nImportsItemsCount;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;

	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumPEImgDelayImports(__in IPE32ImportsCallback* pIPE32ImportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32ImportsCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32ImportsCallback));
		int nImportsItemsCount(0);

		IMAGE_DATA_DIRECTORY& DelayImportDataDir = mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
		if (DelayImportDataDir.Size != 0) 
		{
			__wchar_t lpwszModuleName[MAX_PATH + 1] = { 0 };

			for (ImgDelayDescr* pDelayDesc(reinterpret_cast<ImgDelayDescr*>(GetFilePointer(DelayImportDataDir.VirtualAddress))); pDelayDesc->rvaDLLName != 0; ++pDelayDesc)
			{
				if (pDelayDesc->grAttrs & dlattrRva) 
				{
					char* lptszModuleName = reinterpret_cast<char*>(GetFilePointer(pDelayDesc->rvaDLLName));
					AnsiStrToWideCharStr(lptszModuleName, lpwszModuleName, MAX_PATH, CP_ACP);

					PIMAGE_THUNK_DATA32 pINT(reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pDelayDesc->rvaINT)));
					PIMAGE_THUNK_DATA32 pIAT(reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pDelayDesc->rvaIAT)));
					while (pINT->u1.Ordinal) 
					{
						__PEIMGIMPORTITEM ii;
						ii.dwStructSize = sizeof(__PEIMGIMPORTITEM);
						ii.BoundState = bsUnbounded;
						if (IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal))
						{
							ii.dwOrdinal = pINT->u1.Ordinal & ~IMAGE_ORDINAL_FLAG;
							ii.ImportKind = ikByOrdinal;
							ii.wHint = 0;
						}
						else 
						{
							PIMAGE_IMPORT_BY_NAME p(reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(GetFilePointer(pINT->u1.Ordinal)));
							ii.wHint = p->Hint;
							ii.ImportKind = ikByName;
							ii.pAnsiName = reinterpret_cast<unsigned char*>(p ->Name);
						}
						ii.ForwardState = fsNotForwarded;

						++nImportsItemsCount;

						CComBSTR ModuleName = lpwszModuleName;
						HRESULT hCBResult(pIPE32ImportsCallback ->EnumPE32ImportSymbols(ModuleName.Copy(), &ii, reinterpret_cast<byte*>(pContext)));
						if (FAILED(hCBResult))
						{
							hResult = E_PE32VIEW_CALLBACK_FAILED;
							break;
						}
						if (S_FALSE == hCBResult)
						{
							hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
							break;
						}

						++pINT;
						if (pIAT) 
							++pIAT;
					}
				} 
				else 
				{ 
					// Not tested !!!
					char* lptszModuleName = reinterpret_cast<char*>(GetFilePointer(pDelayDesc->rvaDLLName - mf_lpImgOptHdr ->ImageBase));
					AnsiStrToWideCharStr(lptszModuleName, lpwszModuleName, MAX_PATH, CP_ACP);

					PIMAGE_THUNK_DATA32 pINT(reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pDelayDesc->rvaINT - mf_lpImgOptHdr ->ImageBase)));
					PIMAGE_THUNK_DATA32 pIAT(reinterpret_cast<PIMAGE_THUNK_DATA32>(GetFilePointer(pDelayDesc->rvaIAT - mf_lpImgOptHdr ->ImageBase)));
					while (pINT->u1.Ordinal) 
					{
						__PEIMGIMPORTITEM ii;
						ii.dwStructSize = sizeof(__PEIMGIMPORTITEM);
						ii.BoundState = bsUnbounded;

						if (IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal))
						{
							ii.dwOrdinal = pINT->u1.Ordinal & ~IMAGE_ORDINAL_FLAG;
							ii.ImportKind = ikByOrdinal;
							ii.wHint = 0;
						}
						else 
						{
							PIMAGE_IMPORT_BY_NAME p(reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(GetFilePointer(pINT->u1.Ordinal)));
							ii.wHint = p->Hint;
							ii.ImportKind = ikByName;
							ii.pAnsiName = reinterpret_cast<unsigned char*>(p ->Name);
						}
						ii.ForwardState = fsNotForwarded;
						++nImportsItemsCount;

						CComBSTR ModuleName = lpwszModuleName;
						HRESULT hCBResult(pIPE32ImportsCallback ->EnumPE32ImportSymbols(ModuleName.Copy(), &ii, reinterpret_cast<byte*>(pContext)));
						if (FAILED(hCBResult))
						{
							hResult = E_PE32VIEW_CALLBACK_FAILED;
							break;
						}
						if (S_FALSE == hCBResult)
						{
							hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
							break;
						}

						++pINT; 
						if (pIAT)
							++pIAT;
					}
				}
			}
		}
		else
			hResult = E_PE32VIEW_NO_DELAY_IMPORT;
		if (lpnCallsTotal)
			*lpnCallsTotal = nImportsItemsCount;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;

	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumPEImgCertificates(__in IPE32CertificatesCallback* pIPE32CertificatesCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32CertificatesCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32CertificatesCallback));
		int nCertsCount(0);

		CXFileHandle FileHandle(::CreateFileW(mf_ModuleName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr));
	    if (!FileHandle)
			hResult = HRESULT_FROM_WIN32(::GetLastError());
		else
		{
			DWORD dwCertsCount(0);
			DWORD dwIndices[IMGHLP_ENUM_CERT_BUFFER] = { 0 };
			BOOL fbStatus(::ImageEnumerateCertificates(FileHandle, CERT_SECTION_TYPE_ANY, &dwCertsCount, dwIndices, IMGHLP_ENUM_CERT_BUFFER));
			if (!fbStatus)
				hResult = __HRESULT_FROM_WIN32(::GetLastError());
			else 
				if (dwCertsCount)
				{
					DWORD dwCertLen(0);
					DWORD dwErrorCode;

					for (DWORD i(0); i < dwCertsCount; ++i)
					{
						dwCertLen = 0;
						fbStatus = ::ImageGetCertificateData(FileHandle, dwIndices[i], nullptr, &dwCertLen);
						if (!fbStatus && (ERROR_INSUFFICIENT_BUFFER == (dwErrorCode = ::GetLastError())))
						{
							auto Deleter = [](BYTE* pb) { if (pb) ::CoTaskMemFree(pb); };
							std::unique_ptr<BYTE, decltype(Deleter)> pCert(reinterpret_cast<BYTE*>(::CoTaskMemAlloc(dwCertLen)), Deleter);

							if (pCert)
							{
								fbStatus = ::ImageGetCertificateData(FileHandle, dwIndices[i], reinterpret_cast<WIN_CERTIFICATE*>(pCert.get()), &dwCertLen);
								if (!fbStatus)
									hResult = HRESULT_FROM_WIN32(::GetLastError());
								else
								{
									CRYPT_DATA_BLOB p7Data;
									char szCodeSigningOID[] = szOID_PKIX_KP_CODE_SIGNING;
									p7Data.cbData = dwCertLen - sizeof(DWORD) - sizeof(WORD) - sizeof(WORD);
									p7Data.pbData = reinterpret_cast<WIN_CERTIFICATE*>(pCert.get()) ->bCertificate;
									CXCertStore Store(::CertOpenStore(CERT_STORE_PROV_PKCS7, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, 0, 0, &p7Data));
									if (Store)
									{
										int nCount(0);
										// Populate the key usage structure with the Code Signing OID ...
										LPSTR lpStr;
										CERT_ENHKEY_USAGE keyUsage;
										keyUsage.cUsageIdentifier = 1;
										keyUsage.rgpszUsageIdentifier = &lpStr;
										keyUsage.rgpszUsageIdentifier[0] = &szCodeSigningOID[0];

										PCCERT_CONTEXT  pCertContext(nullptr);

										// Find certificates that contain the Code Signing Enhanced Key Usage ...
										do
										{
											pCertContext = ::CertFindCertificateInStore(Store.Handle(), X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, CERT_FIND_ENHKEY_USAGE, &keyUsage, pCertContext);               		
											if (pCertContext)
											{
												BSTR pwszCertSubject(nullptr);
												GetCertificateSubject(pCertContext, &pwszCertSubject);
												HRESULT hCBResult(pIPE32CertificatesCallback ->EnumPE32Certificates(nCount, pwszCertSubject, reinterpret_cast<BYTE*>(pContext)));
												++nCount;
												if (FAILED(hCBResult))
												{
													hResult = E_PE32VIEW_CALLBACK_FAILED;
													break;
												}
												if (S_FALSE == hCBResult)
												{
													hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
													break;
												}
											}
										} 
										while (pCertContext);

										if (lpnCallsTotal)
											*lpnCallsTotal = nCount;
									}
									else
										hResult = HRESULT_FROM_WIN32(::GetLastError());
								}
							}
							else
								hResult = HRESULT_FROM_WIN32(::GetLastError());
						}
						else
							hResult = __HRESULT_FROM_WIN32(dwErrorCode);
					}
				}
				else
					hResult = E_PE32VIEW_NO_CERTIFICATES;
		}
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;

	return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgDotNETDescriptorPresent(__out_opt LPDOTNETDESC lpDotNETDesc)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size)
	{
		DWORD dwDotNETDescStart(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress);
		PIMAGE_COR20_HEADER pImgNetCorHdr(reinterpret_cast<PIMAGE_COR20_HEADER>(GetFilePointer(dwDotNETDescStart)));
		if (pImgNetCorHdr && (pImgNetCorHdr ->cb == sizeof(IMAGE_COR20_HEADER)))
		{
			if (lpDotNETDesc)
			{
				lpDotNETDesc ->MajorRuntimeVersion = pImgNetCorHdr ->MajorRuntimeVersion;
				lpDotNETDesc ->MinorRuntimeVersion = pImgNetCorHdr ->MinorRuntimeVersion;
				lpDotNETDesc ->dwFlags = pImgNetCorHdr ->Flags;
			}
		}
		else
			hResult = E_PE32VIEW_DOTNET_DESC_UNKNOWN;
	}
	else
		hResult = E_PE32VIEW_NO_DOTNET_DESC;
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumPEImgExports(__in IPE32ExportsCallback* pIPE32ExportsCallback, __in_opt void* pContext, __out_opt int* lpnCallsTotal)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32ExportsCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32ExportsCallback));
		int nExportsCount(0);

		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
		{
			//	Retrieve image exports' directory parameters
			IMAGE_DATA_DIRECTORY& ExportDataDir = mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT];
			DWORD dwExportDirStart(ExportDataDir.VirtualAddress);
			DWORD dwExportDirEnd(dwExportDirStart + ExportDataDir.Size);

			//	Retreive exports table pointer and all relevant tables' ptrs.
			IMAGE_EXPORT_DIRECTORY* pExportDir(reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(GetFilePointer(dwExportDirStart)));
			LPDWORD pAddrTable(reinterpret_cast<LPDWORD>(GetFilePointer(pExportDir->AddressOfFunctions)));
			LPDWORD pNameTable(reinterpret_cast<LPDWORD>(GetFilePointer(pExportDir->AddressOfNames)));
			LPWORD pOrdTable(reinterpret_cast<LPWORD>(GetFilePointer(pExportDir->AddressOfNameOrdinals)));
			//	Whole table iteration.
			for (UINT i(0); i < pExportDir->NumberOfFunctions; ++i) 
			{
				__PEIMGEXPORTITEM ei = { 0 };
				ei.dwStructSize = sizeof(__PEIMGEXPORTITEM);

				DWORD dwRVA(*pAddrTable++);
				//	Skip unused elements.
				if (dwRVA == 0)
					continue;
				ei.dwOrdinal = i + pExportDir->Base;
				//	Check the RVA type.
				if ((dwRVA >= dwExportDirStart) && (dwRVA < dwExportDirEnd))
				{
					ei.ForwardState = fsForwarded;
					ei.pForwardedName = reinterpret_cast<unsigned char*>(GetFilePointer(dwRVA));
				}
				else
				{
					ei.ForwardState = fsNotForwarded;
					ei.pForwardedName = nullptr;
				}

				//	Whole numbers table iteration...
				for (UINT j(0); j < pExportDir->NumberOfNames; j++) 
				{
					if (pOrdTable[j] == i) 
					{
						ei.pAnsiName = reinterpret_cast<unsigned char*>(GetFilePointer(pNameTable[j]));
						break;
					}
				}
				
				++nExportsCount;

				HRESULT hCBResult(pIPE32ExportsCallback ->EnumPE32ExportSymbols(nExportsCount - 1, &ei, reinterpret_cast<byte*>(pContext)));
				if (FAILED(hCBResult))
				{
					hResult = E_PE32VIEW_CALLBACK_FAILED;
					break;
				}
				if (S_FALSE == hCBResult)
				{
					hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
					break;
				}
			}
			if (lpnCallsTotal)
				*lpnCallsTotal = nExportsCount;
		}
		else
			hResult = E_PE32VIEW_NO_EXPORT;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::AllocResourceNameCopy(__in __wchar_t* lpwResName, __in DWORD dwNameLen, __out BSTR* ppwszResNameCopy)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	if (lpwResName && dwNameLen && ppwszResNameCopy)
	{
		BSTR pwszNameCopy(::SysAllocStringLen(nullptr, dwNameLen));
		if (pwszNameCopy)
		{
			::ZeroMemory(pwszNameCopy, (dwNameLen + 1) * sizeof(__wchar_t));
			::CopyMemory(pwszNameCopy, lpwResName, dwNameLen * sizeof(__wchar_t));
			
			//	Test chars ...
			for (DWORD dwWCharIndex(0); dwWCharIndex < dwNameLen; ++dwWCharIndex)
				if (pwszNameCopy[dwWCharIndex] < 32)
					pwszNameCopy[dwWCharIndex] = 32;

			*ppwszResNameCopy = pwszNameCopy;
		}
		else
			hResult = E_PE32VIEW_IMALLOC_FAILED;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::WalkImageResDirectory(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in int& cnItemsCount, __in_opt void* pContext)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEntries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
	BOOL fbKeepGoing(TRUE);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEntries; ++dwcnEntryIndex)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text resource type name 
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir((PIMAGE_RESOURCE_DIRECTORY)(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));
			if (FAILED(hResult = WalkImageResDirectory(pImgResSubDir, dwLevel + 1, dwResourceType, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, pIPE32ResourcesCallback, cnItemsCount, pContext)) || E_PE32VIEW_BREAKED_BY_CALLBACK == hResult)
			{
				fbKeepGoing = FALSE;
				break;
			}
		}
		else
		{
			PIMAGE_RESOURCE_DATA_ENTRY pImgResDataEntry((PIMAGE_RESOURCE_DATA_ENTRY)(GetFilePointer(pImgResEntries[dwcnEntryIndex].OffsetToData + pImgResSection ->VirtualAddress)));
			__PEIMGRESOURCEITEM mri = {0};
			mri.dwStructSize = sizeof(__PEIMGRESOURCEITEM);
			mri.ResIDType = fbResByName ? rtByString : fsByNum;
			mri.dwID = dwID;
			mri.lcid = lcidValue;
			mri.dwCodePage = pImgResDataEntry ->CodePage;
			mri.dwResItemDataSize = pImgResDataEntry ->Size;
			mri.lpResItemDataPtr = reinterpret_cast<BYTE*>(GetFilePointer(pImgResDataEntry ->OffsetToData));
			HRESULT hCBResult(S_OK);
			if (fbResByName)
			{
				if (SUCCEEDED(AllocResourceNameCopy(lpwszResNameCopy, dwResNameLen, &mri.pwszName)))
				{
					if (pIPE32ResourcesCallback)
					{
						++cnItemsCount;
						hCBResult = pIPE32ResourcesCallback ->EnumPE32ResourcesByType(dwResourceType, &mri, reinterpret_cast<BYTE*>(pContext));
					}
				}
			}
			else
			{
				mri.pwszName = nullptr;
				if (pIPE32ResourcesCallback)
				{
					cnItemsCount++;
					hCBResult = pIPE32ResourcesCallback ->EnumPE32ResourcesByType(dwResourceType, &mri, reinterpret_cast<BYTE*>(pContext));
				}
			}

			if (FAILED(hCBResult))
			{
				hResult = E_PE32VIEW_CALLBACK_FAILED;
				fbKeepGoing = FALSE;
				break;
			}
			if (S_FALSE == hCBResult)
			{
				hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
				fbKeepGoing = FALSE;
				break;
			}
		}
		if (!fbKeepGoing)
			break;
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumResByTypeTotal(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in int& cnItemsCount)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEntries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
	BOOL fbKeepGoing(TRUE);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEntries; ++dwcnEntryIndex)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString((PIMAGE_RESOURCE_DIR_STRING_U)(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir((PIMAGE_RESOURCE_DIRECTORY)(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));
			if (FAILED(hResult = EnumResByTypeTotal(pImgResSubDir, dwLevel + 1, dwResourceType, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, cnItemsCount)) || E_PE32VIEW_BREAKED_BY_CALLBACK == hResult)
			{
				fbKeepGoing = FALSE;
				break;
			}
		}
		else
			++cnItemsCount;

		if (!fbKeepGoing)
			break;
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumPEImgResourceTypes(__in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in_opt void* pContext)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32ResourcesCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32ResourcesCallback));

		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
		{
			PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
			BOOL fbRepeat(TRUE);

			UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
			UINT dwcnLevel0EntryIndex(0);

			while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
			{
				CComBSTR ResTypeName;
				HRESULT hCBResult(S_OK);
				if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					PIMAGE_RESOURCE_DIR_STRING_U pRDStr(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>((pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name ^ IMAGE_RESOURCE_NAME_IS_STRING) + reinterpret_cast<UINT_PTR>(pImgResDir)));
					if (pRDStr ->Length)
					{
						auto Deleter = [](__wchar_t* pim) { if (pim) ::CoTaskMemFree(pim); };
						std::unique_ptr<__wchar_t, decltype(Deleter)> pwszResTypeName(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc((pRDStr ->Length + 1) * sizeof(__wchar_t))), Deleter);
						::ZeroMemory(pwszResTypeName.get(), (pRDStr ->Length + 1) * sizeof(__wchar_t));
						::CopyMemory(pwszResTypeName.get(), &pRDStr ->NameString[0], pRDStr ->Length * sizeof(__wchar_t));
						ResTypeName = pwszResTypeName.get();

						hCBResult = pIPE32ResourcesCallback ->EnumPE32ResourcesTypes(pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name, ResTypeName.Detach(), reinterpret_cast<byte*>(pContext));
					}
					else
						hCBResult = pIPE32ResourcesCallback ->EnumPE32ResourcesTypes(pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name, nullptr, reinterpret_cast<byte*>(pContext));
				}
				else
					hCBResult = pIPE32ResourcesCallback ->EnumPE32ResourcesTypes(pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name, nullptr, reinterpret_cast<byte*>(pContext));

				if (FAILED(hCBResult))
				{
					hResult = E_PE32VIEW_CALLBACK_FAILED;
					fbRepeat = FALSE;
					break;
				}
				if (S_FALSE == hCBResult)
				{
					hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
					fbRepeat = FALSE;
					break;
				}
				++dwcnLevel0EntryIndex;
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumPEImgResourcesByType(__in DWORD dwResType, __in IPE32ResourcesCallback* pIPE32ResourcesCallback, __in_opt void* pContext)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32ResourcesCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32ResourcesCallback));

		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
		{

			BOOL fbRepeat(TRUE);		
			int cnResItemsCount(0);

			PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));

			UINT dwcnLevel0Enties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
			UINT dwcnLevel0EntryIndex(0);

			while ((dwcnLevel0EntryIndex < dwcnLevel0Enties) && fbRepeat)
			{
				if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
				{
					DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
					PIMAGE_RESOURCE_DIRECTORY pImgResSubDir((PIMAGE_RESOURCE_DIRECTORY)(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

					if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name == dwResType)
					{
						if (FAILED(hResult = WalkImageResDirectory(pImgResSubDir, 1, dwResType, -1, nullptr, 0, -1, FALSE, pIPE32ResourcesCallback, cnResItemsCount, pContext)))
							fbRepeat = FALSE;
					}
				}
				++dwcnLevel0EntryIndex;
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumImageResStrings(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32StringsCallback* pIPE32StringsCallback, __in int& cnItemsCount, __in_opt void* pContext)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	BOOL fbKeepGoing(TRUE);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEnties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEnties; dwcnEntryIndex++)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));
			EnumImageResStrings(pImgResSubDir, dwLevel + 1, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, pIPE32StringsCallback, cnItemsCount, pContext);
		}
		else
		{
			PIMAGE_RESOURCE_DATA_ENTRY pImgResDataEntry(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(GetFilePointer(pImgResEntries[dwcnEntryIndex].OffsetToData + pImgResSection ->VirtualAddress)));

			DWORD dwStringID(0);
			DWORD dwStringIDBase((dwID == 0) ? 0 : (dwID - 1) * 16);
			LPVOID lpData((LPVOID)(GetFilePointer(pImgResDataEntry ->OffsetToData)));
			LPVOID lpDataBeyond(reinterpret_cast<LPVOID>(reinterpret_cast<DWORD>(lpData) + pImgResDataEntry ->Size));

			DWORD dwStringIndex(0);
			while ((dwStringIndex < 16) && (lpData < lpDataBeyond) && fbKeepGoing)
			{
				DWORD dwStringLen(*(WORD*)(lpData));
				dwStringID = dwStringIDBase + dwStringIndex;
				if (dwStringLen != 0)
				{
					BSTR lpwszTempString(nullptr);
					BYTE* lpStringDataPtr(reinterpret_cast<BYTE*>(reinterpret_cast<DWORD>(lpData) + sizeof(WORD)));
					if (SUCCEEDED(AllocResourceNameCopy(reinterpret_cast<__wchar_t*>(lpStringDataPtr), dwStringLen, &lpwszTempString)))
					{
						++cnItemsCount;

						HRESULT hCBResult(pIPE32StringsCallback ->EnumPE32StringResources(lcidValue, dwStringID, lpStringDataPtr, dwStringLen, lpwszTempString, reinterpret_cast<byte*>(pContext)));
						if (FAILED(hCBResult))
						{
							hResult = E_PE32VIEW_CALLBACK_FAILED;
							fbKeepGoing = FALSE;
							break;
						}
						if (S_FALSE == hCBResult)
						{
							hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
							fbKeepGoing = FALSE;
							break;
						}
					}
					lpData = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD>(lpData) + dwStringLen * sizeof(__wchar_t) + sizeof(WORD));
				}
				else
					lpData = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD>(lpData) + sizeof(WORD));
				++dwStringIndex;
			}
		}
		if (!fbKeepGoing)
			break;
	}
	return (hResult);
}

STDMETHODIMP CPE32Decoder::EnumPEImgResourceStrings(__in IPE32StringsCallback* pIPE32StringsCallback, __in_opt void* pContext)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32StringsCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32StringsCallback));

		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
		{

			BOOL fbRepeat(TRUE);		
			int cnResItemsCount(0);

			PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));

			UINT dwcnLevel0Enties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
			UINT dwcnLevel0EntryIndex(0);

			while ((dwcnLevel0EntryIndex < dwcnLevel0Enties) && fbRepeat)
			{
				if ((pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) != 0)
				{
					DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
					PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

					if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name == RESOURCE_ID_STRING)
						if (FAILED(hResult = EnumImageResStrings(pImgResSubDir, 1, -1, nullptr, 0, -1, FALSE, pIPE32StringsCallback, cnResItemsCount, pContext)))
							fbRepeat = FALSE;

					++dwcnLevel0EntryIndex;
				}
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumImageMsgTableItems(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __in IPE32MessagesCallback* pIPE32MessagesCallback, __in int& cnItemsCount, __in_opt void* pContext)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	BOOL fbKeepGoing(TRUE);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEnties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEnties; dwcnEntryIndex++)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));
			EnumImageMsgTableItems(pImgResSubDir, dwLevel + 1, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, pIPE32MessagesCallback, cnItemsCount, pContext);
		}
		else
		{
			PIMAGE_RESOURCE_DATA_ENTRY pImgResDataEntry(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(GetFilePointer(pImgResEntries[dwcnEntryIndex].OffsetToData + pImgResSection ->VirtualAddress)));
			PMESSAGE_RESOURCE_DATA lpMsgData(reinterpret_cast<PMESSAGE_RESOURCE_DATA>(GetFilePointer(pImgResDataEntry ->OffsetToData)));

			ULONG nCount(lpMsgData ->NumberOfBlocks);
			for (ULONG nIndex(0); nIndex < nCount; ++nIndex)
			{
				if (!fbKeepGoing)
					break;

				PMESSAGE_RESOURCE_ENTRY pMsgEntry(reinterpret_cast<PMESSAGE_RESOURCE_ENTRY>(reinterpret_cast<UINT_PTR>(lpMsgData) + static_cast<UINT_PTR>(lpMsgData ->Blocks[nIndex].OffsetToEntries)));
				for (DWORD dwMsgID(lpMsgData ->Blocks[nIndex].LowId); dwMsgID <= lpMsgData ->Blocks[nIndex].HighId; ++dwMsgID)
				{
					if (!fbKeepGoing)
						break;

					WORD nLen(pMsgEntry ->Length);
					HRESULT hCBResult;
					CComBSTR StrBuf;
					StrBuf.Attach(::SysAllocStringLen(nullptr, nLen));
					if (StrBuf.m_str)
					{
						::ZeroMemory(StrBuf.m_str, (nLen + 1) * sizeof(__wchar_t));
						if (MESSAGE_RESOURCE_UNICODE != pMsgEntry ->Flags)
						{
							auto Deleter1 = [](char* pch) { if (pch) ::CoTaskMemFree(pch); };
							std::unique_ptr<char, decltype(Deleter1)> Buffer(reinterpret_cast<char*>(::CoTaskMemAlloc(sizeof(char) * (nLen + 1))), Deleter1);

							if (Buffer)
							{
								::CopyMemory(Buffer.get(), &pMsgEntry ->Text[0], nLen * sizeof(char));
								if (SUCCEEDED(AnsiStrToWideCharStr(Buffer.get(), StrBuf.m_str, nLen, CP_ACP)))
								{
									hCBResult = pIPE32MessagesCallback ->EnumPE32MsgTableItemsResources(lcidValue, dwMsgID, &pMsgEntry ->Text[0], dwID, nLen, StrBuf.Detach(), reinterpret_cast<byte*>(pContext));
									if (FAILED(hCBResult))
									{
										hResult = E_PE32VIEW_CALLBACK_FAILED;
										fbKeepGoing = FALSE;
										break;
									}
									if (S_FALSE == hCBResult)
									{
										hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
										fbKeepGoing = FALSE;
										break;
									}
								}
							}
						}
						else
						{
							::CopyMemory(StrBuf.m_str, &pMsgEntry ->Text[0], nLen * sizeof(__wchar_t));
							hCBResult = pIPE32MessagesCallback ->EnumPE32MsgTableItemsResources(lcidValue, dwMsgID, reinterpret_cast<BYTE*>(&pMsgEntry ->Text[0]), dwID, nLen, StrBuf.Detach(), reinterpret_cast<byte*>(pContext));
							if (FAILED(hCBResult))
							{
								hResult = E_PE32VIEW_CALLBACK_FAILED;
								fbKeepGoing = FALSE;
								break;
							}
							if (S_FALSE == hCBResult)
							{
								hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
								fbKeepGoing = FALSE;
								break;
							}
						}
					}
					pMsgEntry = reinterpret_cast<PMESSAGE_RESOURCE_ENTRY>(reinterpret_cast<DWORD>(pMsgEntry) + pMsgEntry ->Length);
				}
			}
		}
		if (!fbKeepGoing)
			break;
	}
	return (hResult);
}

STDMETHODIMP CPE32Decoder::EnumPEImgMessageTableItems(__in IPE32MessagesCallback* pIPE32MessagesCallback, __in_opt void* pContext)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pIPE32MessagesCallback)
	{
		CInterfaceMarker IMarker(dynamic_cast<IUnknown*>(pIPE32MessagesCallback));

		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
		{

			BOOL fbRepeat(TRUE);		
			int cnResItemsCount(0);

			PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));

			UINT dwcnLevel0Enties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
			UINT dwcnLevel0EntryIndex(0);

			while ((dwcnLevel0EntryIndex < dwcnLevel0Enties) && fbRepeat)
			{
				if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
				{
					DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
					PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

					if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name == RESOURCE_ID_MESSAGETABLE)
						if (FAILED(hResult = EnumImageMsgTableItems(pImgResSubDir, 1, -1, nullptr, 0, -1, FALSE, pIPE32MessagesCallback, cnResItemsCount, pContext)))
							fbRepeat = FALSE;

					++dwcnLevel0EntryIndex;
				}
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgContainsVersion(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult;

	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		hResult = E_PE32VIEW_NO_VERSION;
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		while (dwcnLevel0EntryIndex < dwcnLevel0Entries)
		{
			if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY && RESOURCE_ID_VERSION == pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name)
			{
				hResult = E_PE32VIEW_SUCCESS;
				break;
			}
			++dwcnLevel0EntryIndex;
		}
	}
	else
		hResult = E_PE32VIEW_NO_RESOURCES;

	return hResult;
}

STDMETHODIMP CPE32Decoder::GetPEImgVersionFixedFileInfo(__in __notnull VS_VERSIONINFO* pVI, __out_opt VS_FIXEDFILEINFO** ppFFI, __out WORD* pwFFILen)
{
	HRESULT hResult(E_PE32VIEW_NO_VERSION);

	if (ppFFI)
		*ppFFI = nullptr;
	if (pwFFILen)
		*pwFFILen = 0;

	if (pVI && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, pVI ->szKey, VER_FILE_INFO_KEY_SIZE, VerFileInfoKey, VER_FILE_INFO_KEY_SIZE))
	{
		WORD* pTmp(&pVI ->Padding1[0]);
		WORD* pBorder(reinterpret_cast<WORD*>(reinterpret_cast<UINT_PTR>(pVI) + pVI ->wLength));
		WORD nLen(pVI ->wValueLength);

		if (nLen)
		{
			while (*reinterpret_cast<DWORD*>(pTmp) != VS_FIXEDFILEINFO_SIGNATURE && pTmp < pBorder)
				pTmp = Inc<WORD>(pTmp);

			if (*reinterpret_cast<DWORD*>(pTmp) == VS_FIXEDFILEINFO_SIGNATURE)
			{
				if (ppFFI)
					*ppFFI = reinterpret_cast<VS_FIXEDFILEINFO*>(pTmp);
				if (pwFFILen)
					*pwFFILen = nLen;

				hResult = E_PE32VIEW_SUCCESS;
			}
			else
				hResult = E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO;
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::IsVersionInfoValid(__in __notnull VS_VERSIONINFO* pVI)
{
	HRESULT hResult(S_FALSE);
	if (pVI && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, pVI ->szKey, VER_FILE_INFO_KEY_SIZE, VerFileInfoKey, VER_FILE_INFO_KEY_SIZE))
		hResult = S_OK;
	return hResult;
}

STDMETHODIMP CPE32Decoder::FindVersionInfo(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __out_opt LPDWORD lpdwVISize, __out_opt VS_VERSIONINFO** ppVI)
{
	HRESULT hResult(E_PE32VIEW_NO_VERSION);
	BOOL fbKeepGoing(TRUE);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEnties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEnties; dwcnEntryIndex++)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));
			if (FAILED(hResult = FindVersionInfo(pImgResSubDir, dwLevel + 1, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, lpdwVISize, ppVI)))
				break;
		}
		else
		{
			PIMAGE_RESOURCE_DATA_ENTRY pImgResDataEntry(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(GetFilePointer(pImgResEntries[dwcnEntryIndex].OffsetToData + pImgResSection ->VirtualAddress)));
			VS_VERSIONINFO* pVI(reinterpret_cast<VS_VERSIONINFO*>(GetFilePointer(pImgResDataEntry ->OffsetToData)));

			if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pVI ->szKey, -1, VerFileInfoKey, ARRAYSIZE(VerFileInfoKey)))
			{
				if (ppVI)
					*ppVI = pVI;
				if (lpdwVISize)
					*lpdwVISize = pImgResDataEntry ->Size;
				hResult = E_PE32VIEW_SUCCESS;
			}
			else
			{
				if (ppVI)
					*ppVI = nullptr;
				if (lpdwVISize)
					*lpdwVISize = 0;
				hResult = E_PE32VIEW_INVALID_VERSION_INFO;
			}
			break;
		}
		if (!fbKeepGoing)
			break;
	}
	return (hResult);
}

STDMETHODIMP CPE32Decoder::GetPEImgVersionTranslations(__in __notnull VS_VERSIONINFO* pVI, __out_opt LANGANDCODEPAGE** ppvTranslationsPairs, __out_opt WORD* pwnLCPLen)
{
	if (ppvTranslationsPairs)
		*ppvTranslationsPairs = nullptr;
	if (pwnLCPLen)
		*pwnLCPLen = 0;
	
	HRESULT hResult(E_PE32VIEW_NO_VERSION);
	if (pVI && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pVI ->szKey, VER_FILE_INFO_KEY_SIZE, VerFileInfoKey, VER_FILE_INFO_KEY_SIZE))
	{
		WORD* pTmp(&pVI ->Padding1[0]);
		WORD* pBorder(reinterpret_cast<WORD*>(reinterpret_cast<UINT_PTR>(pVI) + pVI ->wLength));
		VS_FIXEDFILEINFO* pFFInfo(nullptr);

		if (pVI ->wValueLength)
		{
			int nPaddingsCount(0);
			while (*reinterpret_cast<DWORD*>(pTmp) != VS_FIXEDFILEINFO_SIGNATURE && pTmp < pBorder)
			{
				++nPaddingsCount;
				pTmp = Inc<WORD>(pTmp);
			}

			if (*reinterpret_cast<DWORD*>(pTmp) == VS_FIXEDFILEINFO_SIGNATURE)
				pFFInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(pTmp);
			else
				hResult = E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO;			
		}	

		if (E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO != hResult)
		{
			WORD nLength(pVI ->wLength - pVI ->wValueLength - 3 * sizeof(WORD));
			pTmp = reinterpret_cast<WORD*>(reinterpret_cast<UINT_PTR>(pTmp) + pVI ->wValueLength);
			PVER_INFO_HDR pSFI(reinterpret_cast<PVER_INFO_HDR>(pTmp));

			while (nLength > sizeof(VER_INFO_HDR))
			{
				WORD nPaddingsCount(0);
				while (CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, STRING_FILE_INFO_KEY_SIZE, StringFileInfoKey, STRING_FILE_INFO_KEY_SIZE) && 
					CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, VAR_FILE_INFO_KEY_SIZE, VarFileInfoKey, VAR_FILE_INFO_KEY_SIZE) && reinterpret_cast<WORD*>(pSFI) < pBorder)
				{
					++nPaddingsCount;
					++pTmp;
					pSFI = reinterpret_cast<PVER_INFO_HDR>(pTmp);
				}

				if (nPaddingsCount)
					nLength -= nPaddingsCount * sizeof(WORD);

				if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, STRING_FILE_INFO_KEY_SIZE, StringFileInfoKey, STRING_FILE_INFO_KEY_SIZE))
				{
					//	Next structure is StringFileInfoKey
					if (nLength > pSFI ->wLength)
						nLength -= pSFI ->wLength;
					else
						nLength = 0;
					//	Adjust pointer to next structure ...
					if (nLength)
					{
						WORD nSFILen(pSFI ->wLength);
						pSFI = reinterpret_cast<PVER_INFO_HDR>(reinterpret_cast<UINT_PTR>(pSFI) + nSFILen);
						pTmp = reinterpret_cast<WORD*>(pSFI);
					}
				}
				else
				{
					//	Next structure is VarFileInfoKey
					PVER_VAR pVar(Align32<VER_VAR>(reinterpret_cast<PVER_VAR>(reinterpret_cast<UINT_PTR>(pSFI) + 3 * sizeof(WORD) + ARRAYSIZE(VarFileInfoKey) * sizeof(__wchar_t))));
					if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pVar ->szKey, TRANSLATION_KEY_SIZE, Translation, TRANSLATION_KEY_SIZE))
					{
						if (ppvTranslationsPairs)
							*ppvTranslationsPairs = Align32<LANGANDCODEPAGE>(reinterpret_cast<PLANGANDCODEPAGE>(reinterpret_cast<UINT_PTR>(pVar) + pVar ->wLength - pVar ->wValueLength));
						if (pwnLCPLen)
							*pwnLCPLen = pVar ->wValueLength / sizeof(LANGANDCODEPAGE);
						hResult = E_PE32VIEW_SUCCESS;
						break;
					}
					else
					{
						hResult = E_PE32VIEW_INVALID_VERSION_INFO;
						break;
					}
				}
			}
		}
	}
	else
		hResult = E_PE32VIEW_NO_RESOURCES;
	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumStringTableString(__in PVER_STRING pVS, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext, __out_opt LPWORD lpwLength)
{
	if (lpwLength)
		*lpwLength = 0;

	HRESULT hResult(E_PE32VIEW_SUCCESS);

	WORD nValueLength(pVS ->wValueLength * sizeof(WORD));
	WORD nKeySize(pVS ->wLength - nValueLength - 3 * sizeof(WORD));
	WORD nKeyLen(nKeySize / sizeof(__wchar_t));

	//	Prepare for key processing ...
	auto Deleter = [](__wchar_t* pim) { if (pim) ::CoTaskMemFree(pim); };
	std::unique_ptr<__wchar_t, decltype(Deleter)> Key(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc((nKeyLen + 1) * sizeof(__wchar_t))), Deleter);
	::ZeroMemory(Key.get(), (nKeyLen + 1) * sizeof(__wchar_t));

	//	Copy key ...
	for (WORD nIndex(0); nIndex < nKeyLen; ++nIndex)
	{
		if (pVS ->szKey[nIndex])
			Key.get()[nIndex] =  pVS ->szKey[nIndex];
		else
			break;
	}

	//	Prepare for value processing;
	__wchar_t* pwszValue(Align32<__wchar_t>(reinterpret_cast<__wchar_t*>(reinterpret_cast<UINT_PTR>(pVS) + pVS ->wLength - nValueLength)));
	std::unique_ptr<__wchar_t, decltype(Deleter)> Value(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(pVS ->wValueLength * sizeof(__wchar_t))), Deleter);
	::ZeroMemory(Value.get(), pVS ->wValueLength * sizeof(__wchar_t));
	
	//	Copy value ...
	for (WORD nIndex(0); nIndex < pVS ->wValueLength - 1; ++nIndex)
	{
		if (pwszValue[nIndex])
			Value.get()[nIndex] = pwszValue[nIndex];
		else
			break;
	}

	CComBSTR bstrKey = Key.get();
	CComBSTR bstrValue = Value.get();

	hResult = pIPE32VersionCallback ->EnumPE32VersionStringTableStrings(wLanguage, wCodePage, bstrKey.Detach(), bstrValue.Detach(), reinterpret_cast<PBYTE>(pContext));

	if (SUCCEEDED(hResult))
	{
		if (S_FALSE == hResult)
			hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
		else
		{
			hResult = E_PE32VIEW_SUCCESS;
		if (lpwLength)
			*lpwLength = pVS ->wLength;
		}
	}
	else
		hResult = E_PE32VIEW_CALLBACK_FAILED;

	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumStringTableContent(__in PVER_STRING_TABLE pST, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext, __out_opt LPWORD lpwLength)
{
	if (lpwLength)
		*lpwLength = 0;

	HRESULT hResult;

		//	Format Lang/CP string
	__wchar_t pwszStringTableKey[STRING_TABLE_KEY_SIZE + 1] = { 0 };
	if (SUCCEEDED(hResult = ::StringCchPrintfW(pwszStringTableKey, STRING_TABLE_KEY_SIZE + 1, L"%04x%04x", wLanguage, wCodePage)))
		hResult = E_PE32VIEW_SUCCESS;
	else
		hResult = E_PE32VIEW_UNEXPECTED_FAULT;

	if (SUCCEEDED(hResult) && pST ->wLength > (sizeof(VER_STRING) + STRING_TABLE_KEY_SIZE * sizeof(__wchar_t)))
	{
		WORD nSTLen(pST ->wLength);
		
		//	Compare string table key with formatted Lang/CP string ...
		if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, pST ->szKey, STRING_TABLE_KEY_SIZE, pwszStringTableKey, STRING_TABLE_KEY_SIZE))
		{
			PVER_STRING pString(Align32<VER_STRING>(reinterpret_cast<PVER_STRING>(reinterpret_cast<UINT_PTR>(&pST ->szKey[0]) + 8 * sizeof(__wchar_t))));
			WORD nStringsSize(pST ->wLength - 3 * sizeof(WORD) - 8 * sizeof(__wchar_t));

			while (nStringsSize > sizeof(VER_STRING_TABLE))
			{
				WORD nLen(0);
				hResult = EnumStringTableString(pString, wLanguage, wCodePage, pIPE32VersionCallback, pContext, &nLen);

				if (E_PE32VIEW_BREAKED_BY_CALLBACK != hResult && SUCCEEDED(hResult))
				{
					if (nStringsSize > nLen)
						nStringsSize -= nLen;
					else
						nStringsSize = 0;

					if (nStringsSize)
					{
						pString = reinterpret_cast<PVER_STRING>(reinterpret_cast<UINT_PTR>(pString) + nLen);
						//pString = Inc<VER_STRING>(pString, nLen);
						WORD nPaddings(0);
						while (reinterpret_cast<UINT_PTR>(pString) & PTR_32BIT_UNALIGN_MASK)
						{
							++nPaddings;
							pString = Inc<VER_STRING>(pString);
						}
						if (nStringsSize > nPaddings)
							nStringsSize -= nPaddings;
						else
							nStringsSize = 0;
					}
				}
				else
					break;
			}

			if ((E_PE32VIEW_BREAKED_BY_CALLBACK != hResult && SUCCEEDED(hResult)) && lpwLength)
				*lpwLength = nSTLen;
		}
		else
			if (SUCCEEDED(hResult) && lpwLength)
				*lpwLength = nSTLen;
	}

	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumStringFileInfo(__in PVER_INFO_HDR pSFI, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (pSFI ->wType == 1 && pSFI ->wLength > (sizeof(VER_STRING_TABLE) + STRING_FILE_INFO_KEY_SIZE * sizeof(__wchar_t)))
	{
		WORD nLength(pSFI ->wLength);
		PVER_STRING_TABLE pStringTable(Align32<VER_STRING_TABLE>(reinterpret_cast<PVER_STRING_TABLE>(reinterpret_cast<UINT_PTR>(&pSFI ->szKey[0]) + STRING_FILE_INFO_KEY_SIZE * sizeof(__wchar_t))));

		nLength -= 3 * sizeof(WORD) + STRING_FILE_INFO_KEY_SIZE * sizeof(__wchar_t);

		while (nLength > (sizeof(VER_INFO_HDR) + STRING_TABLE_KEY_SIZE * sizeof(__wchar_t)) && pSFI ->wType == 1)
		{
			WORD nLen(0);
			hResult = EnumStringTableContent(pStringTable, wLanguage, wCodePage, pIPE32VersionCallback, pContext, &nLen);

			if (E_PE32VIEW_BREAKED_BY_CALLBACK != hResult && SUCCEEDED(hResult))
			{
				if (nLength > nLen)
					nLength -= nLen;
				else
					nLength = 0;
				if (nLength)
					pStringTable = Align32<VER_STRING_TABLE>(reinterpret_cast<PVER_STRING_TABLE>(reinterpret_cast<UINT_PTR>(pStringTable) + nLen));
			}
			else
				break;
		}
	}
	else
		hResult = E_PE32VIEW_NO_RESOURCES;

	return hResult;
}


STDMETHODIMP CPE32Decoder::EnumPEImgVersionStringTableStrings(__in __notnull VS_VERSIONINFO* pVI, __in WORD wLanguage, __in WORD wCodePage, __in IPE32VersionCallback* pIPE32VersionCallback, __in_opt void* pContext)
{
	if (!pIPE32VersionCallback || !pVI)
		return E_PE32VIEW_INVALID_ARGUMENTS;

	HRESULT hResult(E_PE32VIEW_NO_VERSION);
	
	if (pVI && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pVI ->szKey, VER_FILE_INFO_KEY_SIZE, VerFileInfoKey, VER_FILE_INFO_KEY_SIZE))
	{
		WORD* pTmp(&pVI ->Padding1[0]);
		WORD* pBorder(reinterpret_cast<WORD*>(reinterpret_cast<UINT_PTR>(pVI) + pVI ->wLength));
		VS_FIXEDFILEINFO* pFFInfo(nullptr);

		if (pVI ->wValueLength)
		{
			int nPaddingsCount(0);
			while (*reinterpret_cast<DWORD*>(pTmp) != VS_FIXEDFILEINFO_SIGNATURE && pTmp < pBorder)
			{
				++nPaddingsCount;
				pTmp = Inc<WORD>(pTmp);
			}

			if (*reinterpret_cast<DWORD*>(pTmp) == VS_FIXEDFILEINFO_SIGNATURE)
				pFFInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(pTmp);
			else
				hResult = E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO;			
		}	

		if (E_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO != hResult)
		{
			hResult = E_PE32VIEW_SUCCESS;
			WORD nLength(pVI ->wLength - pVI ->wValueLength - 3 * sizeof(WORD));
			pTmp = reinterpret_cast<WORD*>(reinterpret_cast<UINT_PTR>(pTmp) + pVI ->wValueLength);
			PVER_INFO_HDR pSFI(reinterpret_cast<PVER_INFO_HDR>(pTmp));

			while (nLength > (sizeof(VER_INFO_HDR) + max(STRING_FILE_INFO_KEY_SIZE, VAR_FILE_INFO_KEY_SIZE) * sizeof(__wchar_t)))
			{
				WORD nPaddingsCount(0);
				while (CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, STRING_FILE_INFO_KEY_SIZE, StringFileInfoKey, STRING_FILE_INFO_KEY_SIZE) && 
					CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, VAR_FILE_INFO_KEY_SIZE, VarFileInfoKey, VAR_FILE_INFO_KEY_SIZE) && reinterpret_cast<WORD*>(pSFI) < pBorder)
				{
					++nPaddingsCount;
					if (nPaddingsCount > 2)
					{
						nLength = 0;
						hResult = ERROR_PE32VIEW_UNEXPECTED_FAULT;
						break;
					}
					++pTmp;
					pSFI = reinterpret_cast<PVER_INFO_HDR>(pTmp);
				}

				if (nLength)
				{
					if (nPaddingsCount * sizeof(WORD) < nLength)
						nLength -= nPaddingsCount * sizeof(WORD);
					else
						nLength = 0;
				}

				if (nLength > (sizeof(VER_INFO_HDR) + STRING_FILE_INFO_KEY_SIZE * sizeof(__wchar_t)))
				{
					if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pSFI ->szKey, STRING_FILE_INFO_KEY_SIZE, StringFileInfoKey, STRING_FILE_INFO_KEY_SIZE))
					{
						//	Next structure is StringFileInfoKey
						hResult = EnumStringFileInfo(pSFI, wLanguage, wCodePage, pIPE32VersionCallback, pContext);

						if (E_PE32VIEW_BREAKED_BY_CALLBACK == hResult || FAILED(hResult))
							break;

						if (nLength > pSFI ->wLength)
							nLength -= pSFI ->wLength;
						else
							nLength = 0;
						//	Adjust pointer to next structure ...
						if (nLength)
						{
							WORD nSFILen(pSFI ->wLength);
							pSFI = reinterpret_cast<PVER_INFO_HDR>(reinterpret_cast<UINT_PTR>(pSFI) + nSFILen);
							pTmp = reinterpret_cast<WORD*>(pSFI);
						}
					}
					else
					{
						//	Next structure is VarFileInfoKey
						if (nLength > pSFI ->wLength)
							nLength -= pSFI ->wLength;
						else
							nLength = 0;
						//	Adjust pointer to next structure ...
						if (nLength)
						{
							WORD nSFILen(pSFI ->wLength);
							pSFI = reinterpret_cast<PVER_INFO_HDR>(reinterpret_cast<UINT_PTR>(pSFI) + nSFILen);
							pTmp = reinterpret_cast<WORD*>(pSFI);
						}
					}
				}
				else
					break;
			}
		}
	}

	return hResult;
}

STDMETHODIMP CPE32Decoder::EnumTypeLibraries(__in IPE32TypelibCallback* pIPE32TypelibCallback, __in_opt void* pContext)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_NO_RESOURCES);

	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		int iTypeLibsTotal(0);

		//	Try to create dynamic structure array for RES_TYPE_NAME
		HDSA hDSAResNamesArray(::DSA_Create(sizeof(RES_TYPE_NAME), 4));
		if (!hDSAResNamesArray)
			return E_PE32VIEW_UNEXPECTED_FAULT;

		//	Fill the array
		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (0x80000000 == (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & 0x80000000))
			{
				DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
				PIMAGE_RESOURCE_DIRECTORY pImgResSubDir((PIMAGE_RESOURCE_DIRECTORY)(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

				 if (FAILED(hResult = EnumResNamesByType(pImgResSubDir, 1, pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name, -1, nullptr, 0, 0, FALSE, hDSAResNamesArray)))
					 break;
			}

			++dwcnLevel0EntryIndex;
		}

		hResult = E_PE32VIEW_SUCCESS;

		int nNamesTotal(DSA_GetItemCount(hDSAResNamesArray));
		if (nNamesTotal)
		{
			//	Enumerate type libraries from resources' names set ...
			for (int nIndex(0); nIndex < nNamesTotal; ++nIndex)
			{
				CComPtr<ITypeLib> iTL;
				PRES_TYPE_NAME pRTN(reinterpret_cast<PRES_TYPE_NAME>(::DSA_GetItemPtr(hDSAResNamesArray, nIndex)));
				if (pRTN ->pwszFullName)
				{
					if (SUCCEEDED(::LoadTypeLibEx(pRTN ->pwszFullName, REGKIND_NONE, &iTL)))
					{
						pRTN ->State = rsTypeLib;
						++iTypeLibsTotal;
					}
					else
						pRTN ->State = rsNonTypeLib;
				}
				iTL.Release();
			}

			//	Call IPE32TypelibCallback interface method ...
			for (int nIndex(0); nIndex < nNamesTotal; ++nIndex)
			{
				PRES_TYPE_NAME pRTN(reinterpret_cast<PRES_TYPE_NAME>(::DSA_GetItemPtr(hDSAResNamesArray, nIndex)));
				if (rsTypeLib == pRTN ->State)
				{
					CComBSTR Short;
					CComBSTR Full;

					Short.Attach(pRTN ->pwszShortName);
					Full.Attach(pRTN ->pwszFullName);

					__PEIMGRESTYPELIBITEM rtli = { 0 };
					rtli.dwStructSize = sizeof(__PEIMGRESTYPELIBITEM);
					rtli.bstrShortName = Short.Copy();
					rtli.bstrFullName = Full.Copy();

					Short.Detach();
					Full.Detach();

					//	Call the passed interface method ...
					HRESULT hCBResult(pIPE32TypelibCallback ->EnumPE32Typelib(&rtli, reinterpret_cast<PBYTE>(pContext)));

					if (FAILED(hCBResult))
					{
						hResult = E_PE32VIEW_CALLBACK_FAILED;
						fbRepeat = FALSE;
						break;
					}
					if (S_FALSE == hCBResult)
					{
						hResult = E_PE32VIEW_BREAKED_BY_CALLBACK;
						fbRepeat = FALSE;
						break;
					}					
				}
			}

			//	Dispose array ...
			for (int nIndex(0); nIndex < nNamesTotal; ++nIndex)
			{
				PRES_TYPE_NAME pRTN(reinterpret_cast<PRES_TYPE_NAME>(::DSA_GetItemPtr(hDSAResNamesArray, nIndex)));
				if (pRTN ->pwszFullName)
				{
					::SysFreeString(pRTN ->pwszFullName);
					pRTN ->pwszFullName = nullptr;
				}
				if (pRTN ->pwszShortName)
				{
					::SysFreeString(pRTN ->pwszShortName);
					pRTN ->pwszShortName = nullptr;
				}
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;

		::DSA_Destroy(hDSAResNamesArray);
	}

	return hResult;
}

STDMETHODIMP CPE32Decoder::NameIsInList(__in __notnull HDSA hResNamesArray, __in __notnull __wchar_t* const pwszShortName)
{
	HRESULT hR(S_FALSE);
	int nCount;
	if (nCount = DSA_GetItemCount(hResNamesArray))
	{
		for (int nIndex(0); nIndex < nCount; ++nIndex)
		{
			RES_TYPE_NAME ResName = { 0 };
			if (::DSA_GetItem(hResNamesArray, nIndex, &ResName))
			{
				if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, pwszShortName, -1, ResName.pwszShortName, -1))
				{
					hR = S_OK;
					break;
				}
			}
			else
				break;
		}
	}
	return hR;
}

STDMETHODIMP CPE32Decoder::EnumResNamesByType(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in DWORD dwResourceType, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __inout HDSA hResNamesArray)
{
	HRESULT hResult(E_PE32VIEW_SUCCESS);
	BOOL fbKeepGoing(TRUE);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEnties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEnties; dwcnEntryIndex++)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));

			if (FAILED(hResult = EnumResNamesByType(pImgResSubDir, dwLevel + 1, dwResourceType, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, hResNamesArray)))
				break;
		}
		else
		{
			//	Build string to pass to LoadTypeLib ...
			__wchar_t pwszFullBuffer[2 * MAX_PATH] = { 0 };
			__wchar_t pwszShortBuffer[MAX_PATH] = { 0 };

			HRESULT hR;
			if (fbResByName)
			{
				//	1. Create null-terminated string from resource name ...
				if (SUCCEEDED(hR))
					hR = ::StringCchCopyNW(pwszShortBuffer, MAX_PATH, lpwszName, dwNameLen);
				//	2. Create path
				if (SUCCEEDED(hR))
					hR = ::StringCchPrintfW(pwszFullBuffer, 2 * MAX_PATH, L"%s\\%s", mf_ModuleName.m_str, pwszShortBuffer);
			}
			else
			{
				//	 Create path for resource ID ...
				hR = ::StringCchPrintfW(pwszFullBuffer, 2 * MAX_PATH, L"%s\\%u", mf_ModuleName.m_str, dwResID);
				if (SUCCEEDED(hR))
					hR = ::StringCchPrintfW(pwszShortBuffer, MAX_PATH, L"%u", dwResID);
			}

			if (SUCCEEDED(hR))
			{
				CComBSTR bstrFullResName = pwszFullBuffer;
				CComBSTR bstrShortResName = pwszShortBuffer;

				RES_TYPE_NAME ResTypeName = { sizeof(RES_TYPE_NAME), rsUnchecked, bstrShortResName.m_str, bstrFullResName.m_str };

				if (S_FALSE == NameIsInList(hResNamesArray, bstrShortResName.m_str))
				{
					bstrShortResName.Detach();
					bstrFullResName.Detach();

					if (0 > ::DSA_InsertItem(hResNamesArray, DSA_APPEND, &ResTypeName))
					{
						fbKeepGoing = FALSE;
						hResult = E_PE32VIEW_UNEXPECTED_FAULT;
					}
				}
			}
			else
			{
				hResult = hR;
				fbKeepGoing = FALSE;
			}
		}
		if (!fbKeepGoing)
			break;
	}
	return (hResult);
}

STDMETHODIMP CPE32Decoder::IsPEImgContainsTypeLib(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	HRESULT hResult(E_PE32VIEW_NO_RESOURCES);
	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
			{
				PIMAGE_RESOURCE_DIR_STRING_U pRDStr(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>((pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name ^ IMAGE_RESOURCE_NAME_IS_STRING) + reinterpret_cast<UINT_PTR>(pImgResDir)));
				if (pRDStr ->Length >= REGISTRY_RESTYPE_NAME_LEN && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, &pRDStr ->NameString[0], REGISTRY_RESTYPE_NAME_LEN, pwsTypeLibResType, TYPELIB_RESTYPE_NAME_LEN))
				{
					hResult = E_PE32VIEW_SUCCESS;
					fbRepeat = FALSE;
					break;
				}
			}

			++dwcnLevel0EntryIndex;
		}		
	}
	
	if (E_PE32VIEW_NO_RESOURCES == hResult)
	{
		unsigned long ulTypeLibResTotal(0);
		IsPEImgContainsTypeLibEx(&ulTypeLibResTotal);

		if (ulTypeLibResTotal)
			return S_OK;
		else
			return S_FALSE;
	}
	else
		return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgContainsTypeLibEx(__out_opt unsigned long *pulTypeLibResTotal)
{
	if (pulTypeLibResTotal)
		*pulTypeLibResTotal = 0;

	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		int iTypeLibsTotal(0);

		//	Try to create dynamic structure array for RES_TYPE_NAME
		HDSA hDSAResNamesArray(::DSA_Create(sizeof(RES_TYPE_NAME), 4));
		if (!hDSAResNamesArray)
			return E_PE32VIEW_UNEXPECTED_FAULT;

		//	Fill the array
		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (0x80000000 == (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & 0x80000000))
			{
				DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
				PIMAGE_RESOURCE_DIRECTORY pImgResSubDir((PIMAGE_RESOURCE_DIRECTORY)(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

				 if (FAILED(hResult = EnumResNamesByType(pImgResSubDir, 1, pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name, -1, nullptr, 0, 0, FALSE, hDSAResNamesArray)))
					 break;
			}

			++dwcnLevel0EntryIndex;
		}

		int nNamesTotal(DSA_GetItemCount(hDSAResNamesArray));

		//	Enumerate type libraries ...
		if (nNamesTotal)
		{
			for (int nIndex(0); nIndex < nNamesTotal; ++nIndex)
			{
				CComPtr<ITypeLib> iTL;
				PRES_TYPE_NAME pRTN(reinterpret_cast<PRES_TYPE_NAME>(::DSA_GetItemPtr(hDSAResNamesArray, nIndex)));
				if (pRTN ->pwszFullName)
				{
					if (S_OK == ::LoadTypeLibEx(pRTN ->pwszFullName, REGKIND_NONE, &iTL) && iTL.p)
					{
						pRTN ->State = rsTypeLib;
						++iTypeLibsTotal;
					}
					else
						pRTN ->State = rsNonTypeLib;
				}
				iTL.Release();
			}
		}

		//	Dispose array ...
		if (nNamesTotal)
		{
			for (int nIndex(0); nIndex < nNamesTotal; ++nIndex)
			{
				PRES_TYPE_NAME pRTN(reinterpret_cast<PRES_TYPE_NAME>(::DSA_GetItemPtr(hDSAResNamesArray, nIndex)));
				if (pRTN ->pwszFullName)
				{
					::SysFreeString(pRTN ->pwszFullName);
					pRTN ->pwszFullName = nullptr;
				}
				if (pRTN ->pwszShortName)
				{
					::SysFreeString(pRTN ->pwszShortName);
					pRTN ->pwszShortName = nullptr;
				}
			}
		}

		::DSA_Destroy(hDSAResNamesArray);

		if (pulTypeLibResTotal)
			*pulTypeLibResTotal = static_cast<unsigned long>(iTypeLibsTotal);
	}

	return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgContainsManifest(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(S_FALSE);

	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name == RESOURCE_ID_MANIFEST)
			{
				hResult = S_OK;
				fbRepeat = FALSE;
				break;
			}

			++dwcnLevel0EntryIndex;
		}
	}

	return hResult;
}

STDMETHODIMP CPE32Decoder::FindManifest(__in PIMAGE_RESOURCE_DIRECTORY pImgResDir, __in DWORD dwLevel, __in LCID lcid, __in __wchar_t* lpwszName, __in DWORD dwNameLen, __in DWORD dwResID, __in BOOL fbResByName, __out_opt LPDWORD pdwManifestSize, __out_opt void** ppManifest)
{
	HRESULT hResult(E_PE32VIEW_NO_TYPELIB);
	BOOL fbKeepGoing(TRUE);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntries(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
	PIMAGE_SECTION_HEADER pImgResSection(GetSection(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress));
	UINT dwcnEnties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);

	for (UINT dwcnEntryIndex(0); dwcnEntryIndex < dwcnEnties; dwcnEntryIndex++)
	{
		LCID lcidValue(lcid);
		DWORD dwID(dwResID);
		__wchar_t* lpwszResNameCopy(lpwszName);
		DWORD dwResNameLen(dwNameLen);
		BOOL fbResByName(fbResByName);

		// Check the level !!!
		switch(dwLevel)
		{
		case 1:
			{
				//	Text name
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		case 2:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					lcidValue = pImgResEntries[dwcnEntryIndex].Name;
				break;
			}
		default:
			{
				if (pImgResEntries[dwcnEntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
				{
					DWORD dwRVA(pImgResEntries[dwcnEntryIndex].Name & ~IMAGE_RESOURCE_NAME_IS_STRING); 
					PIMAGE_RESOURCE_DIR_STRING_U pImgResDirString(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(GetFilePointer(pImgResSection ->VirtualAddress + dwRVA)));
					lpwszResNameCopy = &pImgResDirString ->NameString[0];
					dwResNameLen = pImgResDirString ->Length;		
					fbResByName = TRUE;
				}
				else
					dwID = pImgResEntries[dwcnEntryIndex].Id;
				break;
			}
		}

		if (pImgResEntries[dwcnEntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			DWORD dwSubDirOffset(pImgResEntries[dwcnEntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
			PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + pImgResSection ->VirtualAddress)));

			if (FAILED(hResult = FindManifest(pImgResSubDir, dwLevel + 1, lcidValue, lpwszResNameCopy, dwResNameLen, dwID, fbResByName, pdwManifestSize, ppManifest)))
			{
				if (ppManifest)
					*ppManifest = nullptr;
				if (pdwManifestSize)
					*pdwManifestSize = 0;

				break;
			}
		}
		else
		{
			//pImgResSection ->VirtualAddress
			PIMAGE_RESOURCE_DATA_ENTRY pImgResDataEntry(reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(GetFilePointer(pImgResEntries[dwcnEntryIndex].OffsetToData + pImgResSection ->VirtualAddress)));
			void* pMR(reinterpret_cast<void*>(GetFilePointer(pImgResDataEntry ->OffsetToData)));

			//	Adjust content pointer
			if (pMR)
			{
				char* pszManifest(reinterpret_cast<char*>(pMR));
				if (*pszManifest != '<')
				{
					while (*pszManifest != '<')
						++pszManifest;
					pMR = pszManifest;
				}
			}

			if (ppManifest)
				*ppManifest = pMR;
			if (pdwManifestSize)
				*pdwManifestSize = pImgResDataEntry ->Size;
			hResult = E_PE32VIEW_SUCCESS;
			break;
		}

		if (!fbKeepGoing)
			break;
	}
	return (hResult);
}

STDMETHODIMP CPE32Decoder::GetManifestContent(__out_opt LPDWORD pdwManifestDataSize, __out_opt void** ppManifestContent)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(E_PE32VIEW_SUCCESS);

	if (ppManifestContent)
	{
		if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
		{

			BOOL fbRepeat(TRUE);		
			int cnResItemsCount(0);

			PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(reinterpret_cast<DWORD>(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));

			UINT dwcnLevel0Enties(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
			UINT dwcnLevel0EntryIndex(0);

			while ((dwcnLevel0EntryIndex < dwcnLevel0Enties) && fbRepeat)
			{
				if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
				{
					DWORD dwSubDirOffset(pImgResEntriesLevel0[dwcnLevel0EntryIndex].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY);
					PIMAGE_RESOURCE_DIRECTORY pImgResSubDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(dwSubDirOffset + mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));

					if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name == RESOURCE_ID_MANIFEST)
					{
						hResult = FindManifest(pImgResSubDir, 1, -1, nullptr, 0, -1, FALSE, pdwManifestDataSize, ppManifestContent);
						fbRepeat = FALSE;
					}
				}
				++dwcnLevel0EntryIndex;
			}
		}
		else
			hResult = E_PE32VIEW_NO_RESOURCES;
	}
	else
		hResult = E_PE32VIEW_INVALID_ARGUMENTS;
	return hResult;
}

STDMETHODIMP CPE32Decoder::IsPEImgContainsRegistryRes(void)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;
	
	HRESULT hResult(S_FALSE);
	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
			{
				PIMAGE_RESOURCE_DIR_STRING_U pRDStr(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>((pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name ^ IMAGE_RESOURCE_NAME_IS_STRING) + reinterpret_cast<UINT_PTR>(pImgResDir)));
				if (pRDStr ->Length >= REGISTRY_RESTYPE_NAME_LEN && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, &pRDStr ->NameString[0], REGISTRY_RESTYPE_NAME_LEN, pwsRegistryResType, REGISTRY_RESTYPE_NAME_LEN))
				{
					hResult = S_OK;
					fbRepeat = FALSE;
					break;
				}
			}

			++dwcnLevel0EntryIndex;
		}		
	}
	return hResult;
}

STDMETHODIMP CPE32Decoder::GetPEImgRegistryResTypeID(__out_opt LPDWORD lpdwRegistryResType)
{
	if (FAILED(mf_hInitResult))
		return mf_hInitResult;

	if (!lpdwRegistryResType)
		return E_PE32VIEW_INVALID_ARGUMENTS;
	
	HRESULT hResult(E_PE32VIEW_NO_RESOURCES);
	if (mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size && mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
	{
		PIMAGE_RESOURCE_DIRECTORY pImgResDir(reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(GetFilePointer(mf_lpImgDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)));
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pImgResEntriesLevel0((PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)(pImgResDir) + sizeof(IMAGE_RESOURCE_DIRECTORY)));
		BOOL fbRepeat(TRUE);

		UINT dwcnLevel0Entries(pImgResDir ->NumberOfNamedEntries + pImgResDir ->NumberOfIdEntries);
		UINT dwcnLevel0EntryIndex(0);

		while ((dwcnLevel0EntryIndex < dwcnLevel0Entries) && fbRepeat)
		{
			if (pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name & IMAGE_RESOURCE_NAME_IS_STRING)
			{
				PIMAGE_RESOURCE_DIR_STRING_U pRDStr(reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>((pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name ^ IMAGE_RESOURCE_NAME_IS_STRING) + reinterpret_cast<UINT_PTR>(pImgResDir)));
				if (pRDStr ->Length >= REGISTRY_RESTYPE_NAME_LEN && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, &pRDStr ->NameString[0], REGISTRY_RESTYPE_NAME_LEN, pwsRegistryResType, REGISTRY_RESTYPE_NAME_LEN))
				{
					*lpdwRegistryResType = pImgResEntriesLevel0[dwcnLevel0EntryIndex].Name;
					hResult = E_PE32VIEW_SUCCESS;
					fbRepeat = FALSE;
					break;
				}
			}

			++dwcnLevel0EntryIndex;
		}		
	}
	return hResult;
}
