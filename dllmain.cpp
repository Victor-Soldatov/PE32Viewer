// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "PE32Viewer_i.h"
#include "dllmain.h"
#include "xdlldata.h"

CPE32ViewerModule _AtlModule;
HINSTANCE hDLLInstance;
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

/*
   BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  
    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            // handle error
        }
    }
    return bIsWow64;
*/

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	BOOL bIsWow64(FALSE);
	LPFN_ISWOW64PROCESS fnIsWow64Process(nullptr);
	switch(dwReason) 
	{ 
		case DLL_PROCESS_ATTACH:
			// Initialize once for each new process.
			// Return FALSE to fail DLL load.
			hDLLInstance = hInstance;
			fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "IsWow64Process"));
			if (fnIsWow64Process)
			{
				if (!fnIsWow64Process(::GetCurrentProcess(),&bIsWow64) || bIsWow64)
					return FALSE;
			}

			break;

		case DLL_THREAD_ATTACH:
			// Do thread-specific initialization.
			break;

		case DLL_THREAD_DETACH:
			// Do thread-specific cleanup.
			break;

		case DLL_PROCESS_DETACH:
			// Perform any necessary cleanup.
			break;
	}
	
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	//hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
