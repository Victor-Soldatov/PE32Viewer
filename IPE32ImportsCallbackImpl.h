#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class ATL_NO_VTABLE CIPE32ImportsCallbackImpl : public IPE32ImportsCallback
{
public:

	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32ImportsCallback)

	//	IPE32ImportsCallback
	STDMETHOD(EnumPE32ImportSymbols)(__in BSTR pwszModuleName, __in __PEIMGIMPORTITEM* pItemDesc, __in byte* pContext)	{	return E_NOTIMPL;	}
};

