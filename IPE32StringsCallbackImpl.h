#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class ATL_NO_VTABLE CIPE32StringsCallbackImpl : public IPE32StringsCallback
{
public:
	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32ResourcesCallback)
	//	IPE32StringsCallback
	STDMETHOD(EnumPE32StringResources)(__in LCID lcid, __in DWORD dwID, __in byte* pResData, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext)		{	return E_NOTIMPL;	}
};

