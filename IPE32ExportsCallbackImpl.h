#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class ATL_NO_VTABLE CIPE32ExportsCallbackImpl :
	public IPE32ExportsCallback
{
public:
	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32ExportsCallback)

	//	IPE32ExportsCallback
	STDMETHOD(EnumPE32ExportSymbols)(__in unsigned long nIndex, __in __PEIMGEXPORTITEM* pItemDesc, __in byte* pContext)			{	return E_NOTIMPL;	}
};

