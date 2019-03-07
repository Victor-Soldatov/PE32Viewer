#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class ATL_NO_VTABLE CIPE32ResourcesCallbackImpl :
	public IPE32ResourcesCallback
{
public:

	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32ResourcesCallback)
	//	IPE32ResourcesCallback
	STDMETHOD(EnumPE32ResourcesTypes)(__in unsigned long dwResType, __in byte* pContext)											{	return E_NOTIMPL;	}
	STDMETHOD(EnumPE32ResourcesByType)(__in unsigned long dwResType, __in __PEIMGRESOURCEITEM* pResItemDesc, __in byte* pContext)	{	return E_NOTIMPL;	}
};

