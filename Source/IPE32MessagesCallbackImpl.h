#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class CIPE32MessagesCallbackImpl : public IPE32MessagesCallback
{
public:
	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32MessagesCallback)
	//	IPE32MessagesCallback
	STDMETHOD(EnumPE32MsgTableItemsResources)(__in LCID lcid, __in DWORD dwMsgID, __in byte* pResData, __in DWORD dwID, __in DWORD dwStringLen, __in BSTR pwszString, __in byte* pContext) 		{	return E_NOTIMPL;	}
};

