#pragma once
#include <atlcom.h>
#include "PE32Viewer_i.h"

class ATL_NO_VTABLE CIPE32CertificatesCallbackImpl :
	public IPE32CertificatesCallback
{
public:
	//	IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL(IPE32CertificatesCallback)

	//IPE32CertificatesCallback
	STDMETHOD(EnumPE32Certificates)(__in unsigned long dwIndex, __in BSTR pwszCertSubject, __in byte* pContext)		{	return E_NOTIMPL;	}
};

