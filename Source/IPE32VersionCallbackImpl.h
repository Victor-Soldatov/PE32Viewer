#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class CIPE32VersionCallbackImpl : public IPE32VersionCallback
{
public:
	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32VersionCallback)
	//	IPE32VersionCallback
	STDMETHOD(EnumPE32VersionStringTableStrings)(__in unsigned short wLanguage, __in unsigned short wCodePage, __in BSTR bstrKey, __in BSTR bstrValue, __in byte* pContext) 		{	return E_NOTIMPL;	}
};

