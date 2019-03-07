#pragma warning( disable : 4995 )

#include "stdafx.h"
#include <WindowsX.h>
#include "PE32Viewer.h"
#include "PE32PropsView.h"
#include "PE32Decoder.h"
#include "PE32Misc.h"
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE

#include <strsafe.h>
#include <memory>
#include <vector>

#define WMI_QUERY_BUFFER_SIZE				128
#define	REG_QUERY_BUFFER_SIZE				128
#define REG_VALUE_NAME_SIZE_MAX				16383
#define SMALL_BUFFER_SIZE					64
#define MIDDLE_BUFFER_SIZE					512

#define WM_ASYNC_TV_ADD_ITEM				WM_APP + 1

typedef enum
{
	siCPU,
	siOS,
	siBaseboard
}
SYS_INFO_ITEM;

const __wchar_t WMI_CPU_Class[] = L"Win32_Processor";
const __wchar_t WMI_OS_Class[] = L"Win32_OperatingSystem";
const __wchar_t WMI_BaseBoard_Class[] = L"Win32_BaseBoard";

typedef struct __tagWMI_CLASS_PROP_DESC
{
	UINT dwMsgID;			//	Resource string ID (display name)
	__wchar_t* pWMIClassPropName;
	LONG cbFieldNo;
}
WMI_CLASS_PROP_DESC, *PWMI_CLASS_PROP_DESC;

typedef struct __tagSYS_INFO_STEP
{
	SYS_INFO_ITEM sii;
	const __wchar_t* const pWMIClassName;
	UINT dwMsgID;			//	Resource string ID (display name)
	DWORD dwPropsTotal;
	const WMI_CLASS_PROP_DESC* const PropDesc;
}
SYS_INFO_STEP, *PSYS_INFO_STEP;

typedef struct __tagCPU_PROPS
{
	BSTR pszName;
	BSTR pszVersion;
	BSTR pszSocket;
}
CPU_PROPS, *PCPU_PROPS;

typedef struct __tagBB_PROPS
{
	BSTR pszManufacturer;
	BSTR pszProduct;
}
BB_PROPS, *PBB_PROPS;

typedef struct __tagOS_PROPS
{
	BSTR pszName;
	BSTR pszVersion;
	BSTR pszManufacturer;
	BSTR pszType;
	BSTR pszArchitecture;
}
OS_PROPS, *POS_PROPS;

const WMI_CLASS_PROP_DESC WMI_CPUProps[] = {
	{ IDS_RES_CPU_NAME, L"Name", 0	},
	{ IDS_RES_CPU_VERSION, L"Version", 1	},
	{ IDS_RES_CPU_SOCKET, L"SocketDesignation", 2	}
};

const WMI_CLASS_PROP_DESC WMI_OSProps[] = {
	{ IDS_RES_OS_NAME, L"Caption", 0	},
	{ IDS_RES_OS_VER, L"Version", 1	},
	{ IDS_RES_OS_ARCH, L"OSArchitecture", 4	},
	{ IDS_RES_OS_TYPE, L"BuildType", 3	},
	{ IDS_RES_BB_MANUFACTURER, L"Manufacturer", 2	}
};

const WMI_CLASS_PROP_DESC WMI_BaseBoardProps[] = {
	{ IDS_RES_BB_MANUFACTURER, L"Manufacturer", 0	},
	{ IDS_RES_BB_PRODUCT, L"Product", 1	}
};

void CALLBACK CPE32PropsView::GatherSysInfoCallback(__inout PTP_CALLBACK_INSTANCE Instance, __inout_opt PVOID Context)
{
	PSYS_INFO_CTX psic(reinterpret_cast<PSYS_INFO_CTX>(Context));
	if (psic && sizeof(SYS_INFO_CTX) == psic ->dwStructSize)
	{
		CDSA<CPU_PROPS>* pcpu_props(reinterpret_cast<CDSA<CPU_PROPS>*>(psic ->CPUVector));
		CDSA<BB_PROPS>* pbb_props(reinterpret_cast<CDSA<BB_PROPS>*>(psic ->BBVector));
		CDSA<OS_PROPS>* pos_props(reinterpret_cast<CDSA<OS_PROPS>*>(psic ->OSVector));

		//	Script structure
		SYS_INFO_STEP WMIScript[] = {
			{	siCPU, WMI_CPU_Class, IDS_RES_CPU, ARRAYSIZE(WMI_CPUProps), WMI_CPUProps	},
			{	siBaseboard, WMI_BaseBoard_Class, IDS_RES_BASEBOARD, ARRAYSIZE(WMI_BaseBoardProps), WMI_BaseBoardProps	},
			{	siOS, WMI_OS_Class, IDS_RES_OS, ARRAYSIZE(WMI_OSProps), WMI_OSProps	}
		};

		HRESULT hR;
		CCOMInitializer COMInit(COINIT_MULTITHREADED);
		if (SUCCEEDED(hR = COMInit.InitializationResult()) && (SUCCEEDED(hR = ::CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr)) || RPC_E_TOO_LATE == hR))
		{
			CComPtr<IWbemLocator> pLoc;
			if (SUCCEEDED(pLoc.CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER)))
			{
				CComPtr<IWbemServices> pSvc;
				if (SUCCEEDED(pLoc->ConnectServer(reinterpret_cast<BSTR>(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, 0, &pSvc.p)))
				{
					hR = ::CoSetProxyBlanket(pSvc.p, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
					if (SUCCEEDED(hR))
					{
						for (int nIndex(0); nIndex < ARRAYSIZE(WMIScript); ++nIndex)
						{
							//	Try to format query ...
							CComPtr<IEnumWbemClassObject> pEnum;
							__wchar_t szWMIQuery[WMI_QUERY_BUFFER_SIZE] = { 0 };

							if (SUCCEEDED(::StringCchPrintfW(szWMIQuery, WMI_QUERY_BUFFER_SIZE, L"SELECT * FROM %s", WMIScript[nIndex].pWMIClassName)) && SUCCEEDED(pSvc ->ExecQuery(L"WQL", szWMIQuery, WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum.p)))
							{
								CComPtr<IWbemClassObject> pclsObj;
								ULONG uReturn(0);

								while (SUCCEEDED(hR = pEnum ->Next(WBEM_INFINITE, 1, &pclsObj.p, &uReturn)) && WBEM_S_FALSE != hR)
								{
									if (!uReturn)
										break;

									VARIANT vtProp;
									::VariantInit(&vtProp);

									switch(WMIScript[nIndex].sii)
									{
									case siCPU:
										{
											CPU_PROPS cpu = { 0 };

											for (unsigned int nPropIndex(0); nPropIndex < WMIScript[nIndex].dwPropsTotal; ++nPropIndex)
											{
												if (SUCCEEDED(pclsObj ->Get(WMIScript[nIndex].PropDesc[nPropIndex].pWMIClassPropName, 0, &vtProp, nullptr, nullptr)) && vtProp.vt == VT_BSTR)
												{
													CComBSTR PropValue;
													PropValue.Attach(vtProp.bstrVal);
													reinterpret_cast<BSTR*>(&cpu)[WMIScript[nIndex].PropDesc[nPropIndex].cbFieldNo] = PropValue.Copy();
													PropValue.Empty();
													::VariantClear(&vtProp);
												}
											}

											if (pcpu_props && pcpu_props ->mf_HDSA)
												DSA_AppendItem(pcpu_props ->mf_HDSA, &cpu);
										}
										break;
									case siOS:
										{
											OS_PROPS os = { 0 };

											for (unsigned int nPropIndex(0); nPropIndex < WMIScript[nIndex].dwPropsTotal; ++nPropIndex)
											{
												if (SUCCEEDED(pclsObj ->Get(WMIScript[nIndex].PropDesc[nPropIndex].pWMIClassPropName, 0, &vtProp, nullptr, nullptr)) && vtProp.vt == VT_BSTR)
												{
													CComBSTR PropValue;
													PropValue.Attach(vtProp.bstrVal);
													reinterpret_cast<BSTR*>(&os)[WMIScript[nIndex].PropDesc[nPropIndex].cbFieldNo] = PropValue.Copy();
													PropValue.Empty();
													::VariantClear(&vtProp);
												}
											}

											if (pos_props && pos_props ->mf_HDSA)
												DSA_AppendItem(pos_props ->mf_HDSA, &os);

										}
										break;
									case siBaseboard:
										{
											BB_PROPS bb = { 0 };
											for (unsigned int nPropIndex(0); nPropIndex < WMIScript[nIndex].dwPropsTotal; ++nPropIndex)
											{
												if (SUCCEEDED(pclsObj ->Get(WMIScript[nIndex].PropDesc[nPropIndex].pWMIClassPropName, 0, &vtProp, nullptr, nullptr)) && vtProp.vt == VT_BSTR)
												{
													CComBSTR PropValue;
													PropValue.Attach(vtProp.bstrVal);
													reinterpret_cast<BSTR*>(&bb)[WMIScript[nIndex].PropDesc[nPropIndex].cbFieldNo] = PropValue.Copy();
													PropValue.Empty();
													::VariantClear(&vtProp);
												}
											}

											if (pbb_props && pbb_props ->mf_HDSA)
												DSA_AppendItem(pbb_props ->mf_HDSA, &bb);

										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (psic && psic ->hEvent)
		::SetEvent(psic ->hEvent);
}

const __wchar_t szProcessorName[] = L"ProcessorNameString";
const __wchar_t szProcessorIdentifier[] = L"Identifier";

const __wchar_t szBaseBoardManufacturer[] = L"BaseBoardManufacturer";
const __wchar_t szBaseBoardProduct[] = L"BaseBoardProduct";

const __wchar_t szProductName[] = L"ProductName";
const __wchar_t szOSMajorVer[] = L"CurrentMajorVersionNumber";
const __wchar_t szOSMinorVer[] = L"CurrentMinorVersionNumber";
const __wchar_t szOSBuildNo[] = L"CurrentBuildNumber";
const __wchar_t szOSType[] = L"CurrentType";

INT_PTR CALLBACK CPE32PropsView::ChildSystemDlgInitProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CPE32PropsView* pView(reinterpret_cast<CPE32PropsView*>(lParam));
			HWND hTree(::GetDlgItem(hDlg, IDC_SYSINFO_TREE));
			
			if (::IsThemeActive() && ::IsAppThemed())
			{
				::SetWindowTheme(hDlg, VSCLASS_AEROWIZARD, nullptr);
				::EnableThemeDialogTexture(hDlg, ETDT_ENABLE);
				::SetWindowTheme(hTree, VSCLASS_EXPLORER, nullptr);
			}

			CResString Line(IDS_RES_CPU);
			HTREEITEM htiCPUBase(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_ROOT, Line.m_bszResString));

			CDSA<CPU_PROPS> cpu_props;
			CDSA<BB_PROPS> bb_props;
			CDSA<OS_PROPS> os_props;

			SYS_INFO_CTX sictx = { 0 };

			sictx.dwStructSize = sizeof(SYS_INFO_CTX);

			sictx.CPUVector = &cpu_props;
			sictx.BBVector = &bb_props;
			sictx.OSVector = &os_props;

			sictx.hDlg = hDlg;
			sictx.hTree = hTree;
			sictx.htiCPUBase = htiCPUBase;
			sictx.pMainDlg = pView;
			sictx.hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);

			BOOL fbTPItem(sictx.hEvent != nullptr);
			if (fbTPItem)
				fbTPItem = ::TrySubmitThreadpoolCallback(GatherSysInfoCallback, &sictx, nullptr);

			CResString Features(IDS_RES_CPU_FEATURES);
			HTREEITEM htiCPU(pView ->AddItemToTreeView(hTree, htiCPUBase, TVI_LAST, Features.m_bszResString));

			if (!fbTPItem)
			{
				//	Read registry ...
				HKEY hKey;
				if (htiCPU && ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey))
				{
					CXRegKey RegKey(hKey);
					//	Allocate buffer for value name ...
					__wchar_t szClassName[MAX_PATH] = { 0 };
					DWORD cchClassName(MAX_PATH);
					DWORD cSubKeys(0);
					DWORD cbMaxSubKey;
					DWORD cchMaxClass;
					DWORD cValues;
					DWORD cchMaxValue;
					DWORD cbMaxValueData;
					DWORD cbSecurityDescriptor;
					FILETIME ftLastWriteTime;
					if (ERROR_SUCCESS == ::RegQueryInfoKeyW(RegKey.Handle(), szClassName, &cchClassName, nullptr, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime))
					{
						auto Deleter = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
						std::unique_ptr<__wchar_t, decltype(Deleter)> pValueBuffer(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) + cbMaxValueData)), Deleter);

						if (pValueBuffer)
						{
							DWORD cbSize(sizeof(__wchar_t) + cbMaxValueData);
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szProcessorName, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString CPUName(IDS_RES_CPU_NAME);
								HTREEITEM htiCPUName(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, CPUName.m_bszResString));
								if (htiCPUName)
								{
									CComBSTR CPUDisplayName = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiCPUName, TVI_LAST, CPUDisplayName);
								}
							}

							cbSize = sizeof(__wchar_t) + cbMaxValueData;
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szProcessorIdentifier, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString CPUVer(IDS_RES_CPU_VERSION);
								HTREEITEM htiCPUVer(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, CPUVer.m_bszResString));
								if (htiCPUVer)
								{
									CComBSTR CPUDisplayVer = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiCPUVer, TVI_LAST, CPUDisplayVer);
								}
							}
						}
					}
					hKey = nullptr;
				}
			}

			CResString Enabled(IDS_RES_ENABLED);
			CResString Disabled(IDS_RES_DISABLED);

			if (::IsProcessorFeaturePresent(PF_CHANNELS_ENABLED))
			{
				CResString Channels(IDS_RES_CPUF_CHANNELS);
				HTREEITEM htiChannels(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, Channels.m_bszResString));
				if (htiChannels)
					pView ->AddItemToTreeView(hTree, htiChannels, TVI_LAST, Enabled.m_bszResString);
			}

			CResString FPU(IDS_RES_FPU);
			HTREEITEM htiFPU(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, FPU.m_bszResString));
			if (!::IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED))
			{
				if (htiFPU)
				{
					pView ->AddItemToTreeView(hTree, htiFPU, TVI_LAST, Enabled.m_bszResString);
					CResString FPUPrecErrata(IDS_RES_FPU_ERRATA);
					if (::IsProcessorFeaturePresent(PF_FLOATING_POINT_PRECISION_ERRATA))
						pView ->AddItemToTreeView(hTree, htiFPU, TVI_LAST, FPUPrecErrata.m_bszResString);
				}
			}
			else
			{
				if (htiFPU)
					pView ->AddItemToTreeView(hTree, htiFPU, TVI_LAST, Enabled.m_bszResString);
			}

			CResString NX(IDS_RES_NX);
			HTREEITEM htiNX(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, NX.m_bszResString));
			if (::IsProcessorFeaturePresent(PF_NX_ENABLED))
			{
				if (htiNX)
					pView ->AddItemToTreeView(hTree, htiNX, TVI_LAST, Enabled.m_bszResString);
			}
			else
			{
				if (htiNX)
					pView ->AddItemToTreeView(hTree, htiNX, TVI_LAST, Disabled.m_bszResString);
			}

			CResString PAE(IDS_RES_PAE);
			HTREEITEM htiPAE(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, PAE.m_bszResString));
			if (::IsProcessorFeaturePresent(PF_PAE_ENABLED))
			{
				if (htiPAE)
					pView ->AddItemToTreeView(hTree, htiPAE, TVI_LAST, Enabled.m_bszResString);
			}
			else
			{
				if (htiPAE)
					pView ->AddItemToTreeView(hTree, htiPAE, TVI_LAST, Disabled.m_bszResString);
			}

			//	Instructions sets extensions ...
			if (::IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE) || ::IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) || 
				::IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) || ::IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) ||
				::IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
			{
				CResString ExtraSet(IDS_RES_EXTRA_INSTRUCTIONS_SET);
				HTREEITEM htiExtraSet(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, ExtraSet.m_bszResString));

				if (::IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE))
				{
					CResString _3DNow(IDS_RES_3DNOW);
					pView ->AddItemToTreeView(hTree, htiExtraSet, TVI_LAST, _3DNow.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
				{
					CResString MMX(IDS_RES_MMX);
					pView ->AddItemToTreeView(hTree, htiExtraSet, TVI_LAST, MMX.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
				{
					CResString SSE1(IDS_RES_SSE);
					pView ->AddItemToTreeView(hTree, htiExtraSet, TVI_LAST, SSE1.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
				{
					CResString SSE2(IDS_RES_SSE2);
					pView ->AddItemToTreeView(hTree, htiExtraSet, TVI_LAST, SSE2.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE))
				{
					CResString SSE3(IDS_RES_SSE3);
					pView ->AddItemToTreeView(hTree, htiExtraSet, TVI_LAST, SSE3.m_bszResString);
				}
			}

			//	Instructions supported ...
			if (::IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE) || ::IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE128) || ::IsProcessorFeaturePresent(PF_COMPARE64_EXCHANGE128) ||
				::IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE) || ::IsProcessorFeaturePresent(PF_XSAVE_ENABLED))
			{
				CResString Instructions(IDS_RES_IMPL_INSTRUCTIONS);
				HTREEITEM htiInstructions(pView ->AddItemToTreeView(hTree, htiCPU, TVI_LAST, Instructions.m_bszResString));
				if (::IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE))
				{
					CResString cmpxchg(IDS_RES_CMPXCHG);
					pView ->AddItemToTreeView(hTree, htiInstructions, TVI_LAST, cmpxchg.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE128))
				{
					CResString cmpxchg16b(IDS_RES_CMPXCHG16B);
					pView ->AddItemToTreeView(hTree, htiInstructions, TVI_LAST, cmpxchg16b.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_COMPARE64_EXCHANGE128))
				{
					CResString cmp8xchg16(IDS_RES_CMP8XCHG16B);
					pView ->AddItemToTreeView(hTree, htiInstructions, TVI_LAST, cmp8xchg16.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE))
				{
					CResString rdtsc(IDS_RES_RDTSC);
					pView ->AddItemToTreeView(hTree, htiInstructions, TVI_LAST, rdtsc.m_bszResString);
				}
				if (::IsProcessorFeaturePresent(PF_XSAVE_ENABLED))
				{
					CResString xsave(IDS_RES_XSAVE);
					pView ->AddItemToTreeView(hTree, htiInstructions, TVI_LAST, xsave.m_bszResString);
				}
			}

			if (!fbTPItem)
			{
				//	Read registry ...
				HKEY hKey;
				if (ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", 0, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey))
				{
					CXRegKey RegKey(hKey);
					//	Allocate buffer for value name ...
					__wchar_t szClassName[MAX_PATH] = { 0 };
					DWORD cchClassName(MAX_PATH);
					DWORD cSubKeys(0);
					DWORD cbMaxSubKey;
					DWORD cchMaxClass;
					DWORD cValues;
					DWORD cchMaxValue;
					DWORD cbMaxValueData;
					DWORD cbSecurityDescriptor;
					FILETIME ftLastWriteTime;
					if (ERROR_SUCCESS == ::RegQueryInfoKeyW(RegKey.Handle(), szClassName, &cchClassName, nullptr, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime))
					{
						auto Deleter = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
						std::unique_ptr<__wchar_t, decltype(Deleter)> pValueBuffer(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) + cbMaxValueData)), Deleter);

						CResString Baseboard(IDS_RES_BASEBOARD);
						HTREEITEM htiBaseBoard(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, Baseboard.m_bszResString));

						if (htiBaseBoard && pValueBuffer)
						{
							DWORD cbSize(sizeof(__wchar_t) + cbMaxValueData);
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szBaseBoardManufacturer, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString BBManufacturer(IDS_RES_BB_MANUFACTURER);
								HTREEITEM htiBBManufacturer(pView ->AddItemToTreeView(hTree, htiBaseBoard, TVI_LAST, BBManufacturer.m_bszResString));
								if (htiBBManufacturer)
								{
									CComBSTR BBManufacturerDisplayName = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiBBManufacturer, TVI_LAST, BBManufacturerDisplayName);
								}
							}

							cbSize = sizeof(__wchar_t) + cbMaxValueData;
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szBaseBoardProduct, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString BBModel(IDS_RES_BB_PRODUCT);
								HTREEITEM htiBBModel(pView ->AddItemToTreeView(hTree, htiBaseBoard, TVI_LAST, BBModel.m_bszResString));
								if (htiBBModel)
								{
									CComBSTR BBDisplayModel = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiBBModel, TVI_LAST, BBDisplayModel);
								}
							}
						}
					}
					hKey = nullptr;
				}

				if (ERROR_SUCCESS == ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey))
				{
					CXRegKey RegKey(hKey);
					//	Allocate buffer for value name ...
					__wchar_t szClassName[MAX_PATH] = { 0 };
					DWORD cchClassName(MAX_PATH);
					DWORD cSubKeys(0);
					DWORD cbMaxSubKey;
					DWORD cchMaxClass;
					DWORD cValues;
					DWORD cchMaxValue;
					DWORD cbMaxValueData;
					DWORD cbSecurityDescriptor;
					FILETIME ftLastWriteTime;
					if (ERROR_SUCCESS == ::RegQueryInfoKeyW(RegKey.Handle(), szClassName, &cchClassName, nullptr, &cSubKeys, &cbMaxSubKey, &cchMaxClass, &cValues, &cchMaxValue, &cbMaxValueData, &cbSecurityDescriptor, &ftLastWriteTime))
					{
						auto Deleter = [](__wchar_t* psz) { if (psz) ::CoTaskMemFree(psz); };
						std::unique_ptr<__wchar_t, decltype(Deleter)> pValueBuffer(reinterpret_cast<__wchar_t*>(::CoTaskMemAlloc(sizeof(__wchar_t) + cbMaxValueData)), Deleter);

						CResString OS(IDS_RES_OS);
						HTREEITEM htiOS(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, OS.m_bszResString));

						if (htiOS && pValueBuffer)
						{
							DWORD cbSize(sizeof(__wchar_t) + cbMaxValueData);
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szProductName, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString OSName(IDS_RES_OS_NAME);
								HTREEITEM htiOSName(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSName.m_bszResString));
								if (htiOSName)
								{
									CComBSTR OSDisplayName = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiOSName, TVI_LAST, OSDisplayName);
								}
							}

							OSVERSIONINFOEXW osvx = { 0 };
							osvx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
							if (::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osvx)))
							{
								__wchar_t szOSVerBuffer[SMALL_BUFFER_SIZE] = { 0 };
								if (SUCCEEDED(::StringCchPrintfW(szOSVerBuffer, SMALL_BUFFER_SIZE, L"%d.%d.%d", osvx.dwMajorVersion, osvx.dwMinorVersion, osvx.dwBuildNumber)))
								{
									CResString OSVer(IDS_RES_OS_VER);
									HTREEITEM htiOSVer(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSVer.m_bszResString));
									if (htiOSVer)
									{
										CComBSTR OSVerValue = szOSVerBuffer;
										pView ->AddItemToTreeView(hTree, htiOSVer, TVI_LAST, OSVerValue);
									}
								}
							}
							else
							{
								DWORD dwOSMajorVer(0), dwOSMinorVer(0), dwOSMajorVerSize(sizeof(DWORD)), dwOSMinorVerSize(sizeof(DWORD));
								cbSize = sizeof(__wchar_t) + cbMaxValueData;

								if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szOSMajorVer, nullptr, nullptr, reinterpret_cast<LPBYTE>(&dwOSMajorVer), &dwOSMajorVerSize) && 
									ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szProductName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&dwOSMajorVer), &dwOSMinorVerSize) &&
									ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szProductName, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
								{
									__wchar_t szOSVerBuffer[SMALL_BUFFER_SIZE] = { 0 };
									if (SUCCEEDED(::StringCchPrintfW(szOSVerBuffer, SMALL_BUFFER_SIZE, L"%d.%d.%s", dwOSMajorVer, dwOSMinorVer, szOSVerBuffer)))
									{
										CResString OSVer(IDS_RES_OS_VER);
										HTREEITEM htiOSVer(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSVer.m_bszResString));
										if (htiOSVer)
										{
											CComBSTR OSVerValue = szOSVerBuffer;
											pView ->AddItemToTreeView(hTree, htiOSVer, TVI_LAST, OSVerValue);
										}
									}
								}
							}

							cbSize = sizeof(__wchar_t) + cbMaxValueData;
							if (ERROR_SUCCESS == ::RegQueryValueExW(RegKey.Handle(), szOSType, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValueBuffer.get()), &cbSize))
							{
								CResString OSType(IDS_RES_OS_TYPE);
								HTREEITEM htiOSType(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSType.m_bszResString));
								if (htiOSType)
								{
									CComBSTR OSTypeDisplayName = pValueBuffer.get();
									pView ->AddItemToTreeView(hTree, htiOSType, TVI_LAST, OSTypeDisplayName);
								}
							}
						}
					}
					hKey = nullptr;
				}
			}

			if (fbTPItem)
			{
				::WaitForSingleObject(sictx.hEvent, INFINITE);
				::CloseHandle(sictx.hEvent);

				if (0 < DSA_GetItemCount(cpu_props.mf_HDSA))
				{
					CResString CPUName(IDS_RES_CPU);
					if (DSA_GetItemCount(cpu_props.mf_HDSA) > 1)
					{
						PCPU_PROPS pcpu_props;
						for (int nIndex(0); nIndex < DSA_GetItemCount(cpu_props.mf_HDSA); ++nIndex)
						{
							pcpu_props = reinterpret_cast<PCPU_PROPS>(::DSA_GetItemPtr(cpu_props.mf_HDSA, nIndex));
							if (!pcpu_props)
								continue;

							__wchar_t szCPUUnit[SMALL_BUFFER_SIZE] = { 0 };
							if (SUCCEEDED(::StringCchPrintfW(szCPUUnit, SMALL_BUFFER_SIZE, L"%s %d", CPUName.m_bszResString.m_str, nIndex)))
							{
								CComBSTR CPUUnitItem = szCPUUnit;
								HTREEITEM htiCPUUnit(pView ->AddItemToTreeView(hTree, htiCPUBase, TVI_LAST, CPUUnitItem));
								if (htiCPUUnit)
								{
									//	Multiply items ...
									if (pcpu_props ->pszName)
									{
										CResString CPUName(IDS_RES_CPU_NAME);
										HTREEITEM htiCPUName(pView ->AddItemToTreeView(hTree, htiCPUUnit, TVI_LAST, CPUName.m_bszResString));
										if (htiCPUName)
										{
											CComBSTR DisplayName;
											DisplayName.Attach(pcpu_props ->pszName);
											pView ->AddItemToTreeView(hTree, htiCPUName, TVI_LAST, DisplayName);
											DisplayName.Empty();
											pcpu_props ->pszName = nullptr;
										}
										if (pcpu_props ->pszName)
										{
											::SysFreeString(pcpu_props ->pszName);
											pcpu_props ->pszName = nullptr;
										}
									}

									if (pcpu_props ->pszVersion)
									{
										CResString CPUVer(IDS_RES_CPU_VERSION);
										HTREEITEM htiCPUVer(pView ->AddItemToTreeView(hTree, htiCPUUnit, TVI_LAST, CPUVer.m_bszResString));
										if (htiCPUVer)
										{
											CComBSTR CPUDisplayVer;
											CPUDisplayVer.Attach(pcpu_props ->pszVersion);
											pView ->AddItemToTreeView(hTree, htiCPUVer, TVI_LAST, CPUDisplayVer);
											CPUDisplayVer.Empty();
											pcpu_props ->pszVersion = nullptr;
										}
										if (pcpu_props ->pszVersion)
										{
											::SysFreeString(pcpu_props ->pszVersion);
											pcpu_props ->pszVersion = nullptr;
										}
									}

									if (pcpu_props ->pszSocket)
									{
										CResString CPUSocket(IDS_RES_CPU_SOCKET);
										HTREEITEM htiCPUSocket(pView ->AddItemToTreeView(hTree, htiCPUUnit, TVI_LAST, CPUSocket.m_bszResString));
										if (htiCPUSocket)
										{
											CComBSTR CPUSocketDisplayName;
											CPUSocketDisplayName.Attach(pcpu_props ->pszSocket);
											pView ->AddItemToTreeView(hTree, htiCPUSocket, TVI_LAST, CPUSocketDisplayName);
											CPUSocketDisplayName.Empty();
											pcpu_props ->pszSocket = nullptr;
										}
										if (pcpu_props ->pszSocket)
										{
											::SysFreeString(pcpu_props ->pszSocket);
											pcpu_props ->pszSocket = nullptr;
										}
									}
								}
							}
						}
					}
					else
					{
						PCPU_PROPS pcpu_props(reinterpret_cast<PCPU_PROPS>(::DSA_GetItemPtr(cpu_props.mf_HDSA, 0)));
						if (pcpu_props)
						{
							//	Only item ...
							if (pcpu_props ->pszName)
							{
								HTREEITEM htiCPUName(pView ->AddItemToTreeView(hTree, htiCPUBase, TVI_LAST, CPUName.m_bszResString));
								if (htiCPUName)
								{
									CComBSTR DisplayName;
									DisplayName.Attach(pcpu_props ->pszName);
									pView ->AddItemToTreeView(hTree, htiCPUName, TVI_LAST, DisplayName);
									DisplayName.Empty();
									pcpu_props ->pszName = nullptr;
								}
								if (pcpu_props ->pszName)
								{
									::SysFreeString(pcpu_props ->pszName);
									pcpu_props ->pszName = nullptr;
								}
							}

							if (pcpu_props ->pszVersion)
							{
								CResString CPUVer(IDS_RES_CPU_VERSION);
								HTREEITEM htiCPUVer(pView ->AddItemToTreeView(hTree, htiCPUBase, TVI_LAST, CPUVer.m_bszResString));
								if (htiCPUVer)
								{
									CComBSTR CPUDisplayVer;
									CPUDisplayVer.Attach(pcpu_props ->pszVersion);
									pView ->AddItemToTreeView(hTree, htiCPUVer, TVI_LAST, CPUDisplayVer);
									CPUDisplayVer.Empty();
									pcpu_props ->pszVersion = nullptr;
								}
								if (pcpu_props ->pszVersion)
								{
									::SysFreeString(pcpu_props ->pszVersion);
									pcpu_props ->pszVersion = nullptr;
								}
							}

							if (pcpu_props ->pszSocket)
							{
								CResString CPUSocket(IDS_RES_CPU_SOCKET);
								HTREEITEM htiCPUSocket(pView ->AddItemToTreeView(hTree, htiCPUBase, TVI_LAST, CPUSocket.m_bszResString));
								if (htiCPUSocket)
								{
									CComBSTR CPUSocketDisplayName;
									CPUSocketDisplayName.Attach(pcpu_props ->pszSocket);
									pView ->AddItemToTreeView(hTree, htiCPUSocket, TVI_LAST, CPUSocketDisplayName);
									CPUSocketDisplayName.Empty();
									pcpu_props ->pszSocket = nullptr;
								}
								if (pcpu_props ->pszSocket)
								{
									::SysFreeString(pcpu_props ->pszSocket);
									pcpu_props ->pszSocket = nullptr;
								}
							}
						}
					}
				}

				if (DSA_GetItemCount(bb_props.mf_HDSA))
				{
					PBB_PROPS pbb_props;
					CResString Baseboard(IDS_RES_BASEBOARD);

					if (DSA_GetItemCount(bb_props.mf_HDSA) > 1)
					{
						for (int nIndex(0); nIndex < DSA_GetItemCount(bb_props.mf_HDSA); ++nIndex)
						{
							pbb_props = reinterpret_cast<PBB_PROPS>(::DSA_GetItemPtr(bb_props.mf_HDSA, nIndex));
							if (!pbb_props)
								continue;

							__wchar_t szBBUnit[SMALL_BUFFER_SIZE] = { 0 };
							if (SUCCEEDED(::StringCchPrintfW(szBBUnit, SMALL_BUFFER_SIZE, L"%s %d", Baseboard.m_bszResString.m_str, nIndex)))
							{
								CComBSTR BBUnitItem = szBBUnit;
								HTREEITEM htiBBUnit(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, BBUnitItem));
								if (htiBBUnit)
								{
									if (pbb_props ->pszManufacturer)
									{
										CResString BBManufacturer(IDS_RES_BB_MANUFACTURER);
										HTREEITEM htiBBManufacturer(pView ->AddItemToTreeView(hTree, htiBBUnit, TVI_LAST, BBManufacturer.m_bszResString));
										if (htiBBManufacturer)
										{
											CComBSTR BBManufacturerDisplayName;
											BBManufacturerDisplayName.Attach(pbb_props ->pszManufacturer);
											pView ->AddItemToTreeView(hTree, htiBBManufacturer, TVI_LAST, BBManufacturerDisplayName);
											BBManufacturerDisplayName.Empty();
											pbb_props ->pszManufacturer = nullptr;
										}
										if (pbb_props ->pszManufacturer)
										{
											::SysFreeString(pbb_props ->pszManufacturer);
											pbb_props ->pszManufacturer = nullptr;
										}
									}

									if (pbb_props ->pszProduct)
									{
										CResString BBModel(IDS_RES_BB_PRODUCT);
										HTREEITEM htiBBModel(pView ->AddItemToTreeView(hTree, htiBBUnit, TVI_LAST, BBModel.m_bszResString));
										if (htiBBModel)
										{
											CComBSTR BBDisplayModel;
											BBDisplayModel.Attach(pbb_props ->pszProduct);
											pView ->AddItemToTreeView(hTree, htiBBModel, TVI_LAST, BBDisplayModel);
											BBDisplayModel.Empty();
											pbb_props ->pszProduct = nullptr;
										}
										if (pbb_props ->pszProduct)
										{
											::SysFreeString(pbb_props ->pszProduct);
											pbb_props ->pszProduct = nullptr;
										}
									}
								}
							}
						}
					}
					else
					{
						//	Only item ...
						pbb_props = reinterpret_cast<PBB_PROPS>(::DSA_GetItemPtr(bb_props.mf_HDSA, 0));
						if (pbb_props)
						{
							HTREEITEM htiBaseBoard(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, Baseboard.m_bszResString));
							if (pbb_props ->pszManufacturer)
							{
								CResString BBManufacturer(IDS_RES_BB_MANUFACTURER);
								HTREEITEM htiBBManufacturer(pView ->AddItemToTreeView(hTree, htiBaseBoard, TVI_LAST, BBManufacturer.m_bszResString));
								if (htiBBManufacturer)
								{
									CComBSTR BBManufacturerDisplayName;
									BBManufacturerDisplayName.Attach(pbb_props ->pszManufacturer);
									pView ->AddItemToTreeView(hTree, htiBBManufacturer, TVI_LAST, BBManufacturerDisplayName);
									BBManufacturerDisplayName.Empty();
									pbb_props ->pszManufacturer = nullptr;
								}
								if (pbb_props ->pszManufacturer)
								{
									::SysFreeString(pbb_props ->pszManufacturer);
									pbb_props ->pszManufacturer = nullptr;
								}
							}

							if (pbb_props ->pszProduct)
							{
								CResString BBModel(IDS_RES_BB_PRODUCT);
								HTREEITEM htiBBModel(pView ->AddItemToTreeView(hTree, htiBaseBoard, TVI_LAST, BBModel.m_bszResString));
								if (htiBBModel)
								{
									CComBSTR BBDisplayModel;
									BBDisplayModel.Attach(pbb_props ->pszProduct);
									pView ->AddItemToTreeView(hTree, htiBBModel, TVI_LAST, BBDisplayModel);
									BBDisplayModel.Empty();
									pbb_props ->pszProduct = nullptr;
								}
								if (pbb_props ->pszProduct)
								{
									::SysFreeString(pbb_props ->pszProduct);
									pbb_props ->pszProduct = nullptr;
								}
							}
						}
					}
				}

				if (DSA_GetItemCount(os_props.mf_HDSA))
				{
					POS_PROPS pos_props;
					CResString OS(IDS_RES_OS);

					if (DSA_GetItemCount(os_props.mf_HDSA) > 1)
					{
						for (int nIndex(0); nIndex < DSA_GetItemCount(os_props.mf_HDSA); ++nIndex)
						{
							pos_props = reinterpret_cast<POS_PROPS>(::DSA_GetItemPtr(os_props.mf_HDSA, nIndex));
							if (!pos_props)
								continue;

							__wchar_t szOSUnit[SMALL_BUFFER_SIZE] = { 0 };
							if (SUCCEEDED(::StringCchPrintfW(szOSUnit, SMALL_BUFFER_SIZE, L"%s %d", OS.m_bszResString.m_str, nIndex)))
							{
								CComBSTR OSUnitItem = szOSUnit;
								HTREEITEM htiOSUnit(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, OSUnitItem));
								if (htiOSUnit)
								{
									if (pos_props ->pszName)
									{
										CResString OSName(IDS_RES_OS_NAME);
										HTREEITEM htiOSName(pView ->AddItemToTreeView(hTree, htiOSUnit, TVI_LAST, OSName.m_bszResString));
										if (htiOSName)
										{
											CComBSTR OSDisplayName;
											OSDisplayName.Attach(pos_props ->pszName);
											pView ->AddItemToTreeView(hTree, htiOSName, TVI_LAST, OSDisplayName);
											OSDisplayName.Empty();
											pos_props ->pszName = nullptr;
										}
										if (pos_props ->pszName)
										{
											::SysFreeString(pos_props ->pszName);
											pos_props ->pszName = nullptr;
										}
									}

									if (pos_props ->pszVersion)
									{
										CResString OSVer(IDS_RES_OS_VER);
										HTREEITEM htiOSVer(pView ->AddItemToTreeView(hTree, htiOSUnit, TVI_LAST, OSVer.m_bszResString));
										if (htiOSVer)
										{
											CComBSTR OSDisplayVer;
											OSDisplayVer.Attach(pos_props ->pszVersion);
											pView ->AddItemToTreeView(hTree, htiOSVer, TVI_LAST, OSDisplayVer);
											OSDisplayVer.Empty();
											pos_props ->pszVersion = nullptr;
										}
										if (pos_props ->pszVersion)
										{
											::SysFreeString(pos_props ->pszVersion);
											pos_props ->pszVersion = nullptr;
										}
									}

									if (pos_props ->pszManufacturer)
									{
										CResString OSManufacturer(IDS_RES_BB_MANUFACTURER);
										HTREEITEM htiOSManufacturer(pView ->AddItemToTreeView(hTree, htiOSUnit, TVI_LAST, OSManufacturer.m_bszResString));
										if (htiOSManufacturer)
										{
											CComBSTR OSManufacturer;
											OSManufacturer.Attach(pos_props ->pszManufacturer);
											pView ->AddItemToTreeView(hTree, htiOSManufacturer, TVI_LAST, OSManufacturer);
											OSManufacturer.Empty();
											pos_props ->pszManufacturer = nullptr;
										}
										if (pos_props ->pszManufacturer)
										{
											::SysFreeString(pos_props ->pszManufacturer);
											pos_props ->pszManufacturer = nullptr;
										}
									}

									if (pos_props ->pszType)
									{
										CResString OSType(IDS_RES_OS_TYPE);
										HTREEITEM htiOSType(pView ->AddItemToTreeView(hTree, htiOSUnit, TVI_LAST, OSType.m_bszResString));
										if (htiOSType)
										{
											CComBSTR OSTypeName;
											OSTypeName.Attach(pos_props ->pszType);
											pView ->AddItemToTreeView(hTree, htiOSType, TVI_LAST, OSTypeName);
											OSTypeName.Empty();
											pos_props ->pszType = nullptr;
										}
										if (pos_props ->pszType)
										{
											::SysFreeString(pos_props ->pszType);
											pos_props ->pszType = nullptr;
										}
									}

									if (pos_props ->pszArchitecture)
									{
										CResString OSArch(IDS_RES_OS_ARCH);
										HTREEITEM htiOSArch(pView ->AddItemToTreeView(hTree, htiOSUnit, TVI_LAST, OSArch.m_bszResString));
										if (htiOSArch)
										{
											CComBSTR OSArchName;
											OSArchName.Attach(pos_props ->pszArchitecture);
											pView ->AddItemToTreeView(hTree, htiOSArch, TVI_LAST, OSArchName);
											OSArchName.Empty();
											pos_props ->pszArchitecture = nullptr;
										}
										if (pos_props ->pszArchitecture)
										{
											::SysFreeString(pos_props ->pszArchitecture);
											pos_props ->pszArchitecture = nullptr;
										}
									}
								}
							}
						}
					}
					else
					{
						//	Only item ...
						pos_props = reinterpret_cast<POS_PROPS>(::DSA_GetItemPtr(os_props.mf_HDSA, 0));
						if (pos_props)
						{
							HTREEITEM htiOS(pView ->AddItemToTreeView(hTree, TVI_ROOT, TVI_LAST, OS.m_bszResString));
							if (pos_props ->pszName)
							{
								CResString OSName(IDS_RES_OS_NAME);
								HTREEITEM htiOSName(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSName.m_bszResString));
								if (htiOSName)
								{
									CComBSTR OSDisplayName;
									OSDisplayName.Attach(pos_props ->pszName);
									pView ->AddItemToTreeView(hTree, htiOSName, TVI_LAST, OSDisplayName);
									OSDisplayName.Empty();
									pos_props ->pszName = nullptr;
								}
								if (pos_props ->pszName)
								{
									::SysFreeString(pos_props ->pszName);
									pos_props ->pszName = nullptr;
								}
							}

							if (pos_props ->pszVersion)
							{
								CResString OSVer(IDS_RES_OS_VER);
								HTREEITEM htiOSVer(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSVer.m_bszResString));
								if (htiOSVer)
								{
									CComBSTR OSDisplayVer;
									OSDisplayVer.Attach(pos_props ->pszVersion);
									pView ->AddItemToTreeView(hTree, htiOSVer, TVI_LAST, OSDisplayVer);
									OSDisplayVer.Empty();
									pos_props ->pszVersion = nullptr;
								}
								if (pos_props ->pszVersion)
								{
									::SysFreeString(pos_props ->pszVersion);
									pos_props ->pszVersion = nullptr;
								}
							}

							if (pos_props ->pszManufacturer)
							{
								CResString OSManufacturer(IDS_RES_BB_MANUFACTURER);
								HTREEITEM htiOSManufacturer(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSManufacturer.m_bszResString));
								if (htiOSManufacturer)
								{
									CComBSTR OSManufacturer;
									OSManufacturer.Attach(pos_props ->pszManufacturer);
									pView ->AddItemToTreeView(hTree, htiOSManufacturer, TVI_LAST, OSManufacturer);
									OSManufacturer.Empty();
									pos_props ->pszManufacturer = nullptr;
								}
								if (pos_props ->pszManufacturer)
								{
									::SysFreeString(pos_props ->pszManufacturer);
									pos_props ->pszManufacturer = nullptr;
								}
							}

							if (pos_props ->pszType)
							{
								CResString OSType(IDS_RES_OS_TYPE);
								HTREEITEM htiOSType(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSType.m_bszResString));
								if (htiOSType)
								{
									CComBSTR OSTypeName;
									OSTypeName.Attach(pos_props ->pszType);
									pView ->AddItemToTreeView(hTree, htiOSType, TVI_LAST, OSTypeName);
									OSTypeName.Empty();
									pos_props ->pszType = nullptr;
								}
								if (pos_props ->pszType)
								{
									::SysFreeString(pos_props ->pszType);
									pos_props ->pszType = nullptr;
								}
							}

							if (pos_props ->pszArchitecture)
							{
								CResString OSArch(IDS_RES_OS_ARCH);
								HTREEITEM htiOSArch(pView ->AddItemToTreeView(hTree, htiOS, TVI_LAST, OSArch.m_bszResString));
								if (htiOSArch)
								{
									CComBSTR OSArchName;
									OSArchName.Attach(pos_props ->pszArchitecture);
									pView ->AddItemToTreeView(hTree, htiOSArch, TVI_LAST, OSArchName);
									OSArchName.Empty();
									pos_props ->pszArchitecture = nullptr;
								}
								if (pos_props ->pszArchitecture)
								{
									::SysFreeString(pos_props ->pszArchitecture);
									pos_props ->pszArchitecture = nullptr;
								}
							}
						}
					}
				}
			}

			::MoveWindow(hDlg, pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.top, pView ->mf_rcDisplay.right - pView ->mf_rcDisplay.left, pView ->mf_rcDisplay.bottom - pView ->mf_rcDisplay.top, FALSE);
			RECT rc;
			::SetRectEmpty(&rc);
			::GetClientRect(hDlg, &rc);
			::MoveWindow(hTree, rc.left + ::GetSystemMetrics(SM_CXDLGFRAME), rc.top + ::GetSystemMetrics(SM_CYDLGFRAME), rc.right - rc.left - ::GetSystemMetrics(SM_CXDLGFRAME), rc.bottom - rc.top - ::GetSystemMetrics(SM_CYDLGFRAME), FALSE);
			::SetWindowPos(hDlg, ::GetParent(hDlg), 0, 0, 0, 0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			return 1;
		}
	}
	return 0;
}
