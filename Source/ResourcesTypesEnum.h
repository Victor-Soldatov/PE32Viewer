#pragma once
#include "unknwn.h"
#include <atlcom.h>
//#include "PE32Viewer_i.h"

class CIUnknownImpl : public IUnknown
{
public:
	virtual ULONG STDMETHODCALLTYPE AddRef(void)				{	return 0;	}
	virtual ULONG STDMETHODCALLTYPE Release(void)				{	return 0;	}
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)		{	return E_NOINTERFACE;	}
};

