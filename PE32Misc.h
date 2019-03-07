#pragma once
#include <Windows.h>

template <typename T>
class __declspec(novtable) TCustomAllocator
{
public:
	//	types definitions
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	std::size_t unit_size(void) const		
	{	return sizeof(T);	}

	//	methods prototypes
	virtual T* allocate(std::size_t num, const void* hint = nullptr) = 0;								// (num * sizeof(T))
	virtual void deallocate(T* p, std::size_t num) = 0;													//	delete p

	T* address(T& value) const
	{	return &value;	}

	const T* address(const T& value) const
	{	return &value;	}

	std::size_t max_size(void) const throw()
	{	return (UINT_MAX / unit_size());	}

	void construct(T* p, const T& value)			
	{	
		new((void*)p)T(value);
	}

	void destroy(T* p)
	{	
		p ->~T();	
	}
};

template <typename T>
class CXOleAllocator : public TCustomAllocator<T>
{
public:
	CXOleAllocator(void) throw()								{}
	template <typename T1>
	CXOleAllocator(const CXOleAllocator<T1>&) throw()			{}

	CXOleAllocator(const CXOleAllocator&) throw()				{}
	CXOleAllocator(CXOleAllocator&&) throw()					{}
	CXOleAllocator& operator= (const CXOleAllocator&) throw()	{	return *this;	}
	CXOleAllocator& operator= (CXOleAllocator&&) throw()		{	return *this;	}
	virtual ~CXOleAllocator(void) throw()						{}

	virtual T* allocate(std::size_t num, const void* hint = nullptr)								// (num * sizeof(T))
	{
		return reinterpret_cast<T*>(::CoTaskMemAlloc(num * sizeof(T)));
	}

	virtual void deallocate(T* p, std::size_t num)													//	delete p
	{
		if (p)
			::CoTaskMemFree(p);
	}

	template <typename T1>
	struct rebind
	{	typedef CXOleAllocator<T1> other;	};
};

template <typename T1, typename T2>
bool operator== (const CXOleAllocator<T1>&, const CXOleAllocator<T2>) throw() 
{
	return true;
}

template <typename T1, typename T2>
bool operator!= (const CXOleAllocator<T1>&, const CXOleAllocator<T2>) throw() 
{
	return false;
}

template <typename T, T InvalidHandleValue>
class THandleHolder
{
private:
	T mf_HandleValue;
protected:
	virtual void AssignHandle(__in T Handle)
	{
		mf_HandleValue = Handle;
	}

	virtual void ReleaseHandle(void) = 0;

	virtual void Clear(void)
	{
		if (IsValidHandle())
			ReleaseHandle();
	}
public:
	THandleHolder(void) : mf_HandleValue(InvalidHandleValue)											{}
	THandleHolder(__in T hHandleValue) : mf_HandleValue(hHandleValue)									{}
	THandleHolder(const THandleHolder<T, InvalidHandleValue>& Src) : mf_HandleValue(Src.Duplicate())	{}
	THandleHolder(THandleHolder<T, InvalidHandleValue>&& Src) : mf_HandleValue(Src.mf_HandleValue)		
	{
		Src.mf_HandleValue = reinterpret_cast<T>(InvalidHandleValue);
	}

	THandleHolder<T, InvalidHandleValue>& operator= (const THandleHolder<T, InvalidHandleValue>& Src)
	{
		if (this != &Src)
		{
			Clear();
			AssignHandle(Src.Duplicate());
		}
		return *this;
	}

	THandleHolder<T, InvalidHandleValue>& operator= (THandleHolder<T, InvalidHandleValue>&& Src)
	{
		if (this != &Src)
		{
			Clear();
			mf_HandleValue = Src.mf_HandleValue;
			Src.mf_HandleValue = InvalidHandleValue;
		}
		return *this;
	}

	void Assign(__in T Handle)
	{
		Clear();
		AssignHandle(Handle);
	}

	typedef T HandleType;

	virtual T Handle(void) const throw()
	{
		return mf_HandleValue;
	}

	virtual T Duplicate(void) const = 0;

	virtual T Leave(void)
	{
		T Handle(Handle());
		AssignHandle(InvalidHandleValue);
		return Handle;
	}
	
	virtual bool IsValidHandle(void) const
	{
		if (InvalidHandleValue != Handle())
			return true;
		else
			return false;
	}

	virtual ~THandleHolder(void)										{}

	operator T(void) const throw()										{	return Handle();	}
	operator bool(void) const throw()									
	{
		if (InvalidHandleValue != Handle())
			return true;
		else
			return false;
	}
};

class CXFileHandle : public THandleHolder<HANDLE, INVALID_HANDLE_VALUE>
{
protected:
	virtual void ReleaseHandle(void)
	{
		if (nullptr != Handle())
			::CloseHandle(Handle());
	}

	virtual void Clear(void)
	{
		if (nullptr != Handle() && IsValidHandle())
			ReleaseHandle();
	}
public:
	virtual HANDLE Duplicate(void) const
	{
		return INVALID_HANDLE_VALUE;
	}

	CXFileHandle(void) : THandleHolder<HANDLE, INVALID_HANDLE_VALUE>()									{}
	CXFileHandle(__in HANDLE hHandleValue) : THandleHolder<HANDLE, INVALID_HANDLE_VALUE>(hHandleValue)	{}
	virtual ~CXFileHandle(void)																			{	Clear();	}

	operator bool(void) const throw()									
	{
		if (nullptr != (HANDLE)(*this) && INVALID_HANDLE_VALUE != (HANDLE)(*this))
			return true;
		else
			return false;
	}
};

class CXHandle : public THandleHolder<HANDLE, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::CloseHandle(Handle());
	}

public:
	virtual HANDLE Duplicate(void) const
	{
		return nullptr;
	}

	CXHandle(void) : THandleHolder<HANDLE, nullptr>()													{}
	CXHandle(__in HANDLE hHandleValue) : THandleHolder<HANDLE, nullptr>(hHandleValue)					{}
	virtual ~CXHandle(void)																				{	if (Handle()) ReleaseHandle();	}
};

class CXUSKEY : public THandleHolder<HUSKEY, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::SHRegCloseUSKey(Handle());
	}
public:
	virtual HUSKEY Duplicate(void) const
	{
		return nullptr;
	}

	CXUSKEY(void) : THandleHolder<HUSKEY, nullptr>()													{}
	CXUSKEY(__in HUSKEY hUSKey) : THandleHolder<HUSKEY, nullptr>(hUSKey)								{}
	virtual ~CXUSKEY(void)																				{	if (Handle()) ReleaseHandle();	}
};

class CXRegKey : public THandleHolder<HKEY, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::RegCloseKey(Handle());
	}
public:
	virtual HKEY Duplicate(void) const
	{
		return nullptr;
	}

	CXRegKey(void) : THandleHolder<HKEY, nullptr>()														{}
	CXRegKey(__in HKEY hKey) : THandleHolder<HKEY, nullptr>(hKey)										{}
	virtual ~CXRegKey(void)																				{	if (Handle()) ReleaseHandle();	}
	void Close(void)																					{	if (Handle()) {	ReleaseHandle(); AssignHandle(nullptr);	}	}
};

class CXMapView : public THandleHolder<LPVOID, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::UnmapViewOfFile(Handle());
	}

public:
	virtual HANDLE Duplicate(void) const
	{
		return nullptr;
	}

	virtual ~CXMapView(void)																			{	if (Handle()) ReleaseHandle();	}

	CXMapView(void) : THandleHolder<LPVOID, nullptr>()													{}
	CXMapView(__in LPVOID pView) : THandleHolder<LPVOID, nullptr>(pView)								{}
};

class CXCertStore : public THandleHolder<HCERTSTORE, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::CertCloseStore(Handle(), CERT_CLOSE_STORE_FORCE_FLAG);
	}
public:
	virtual HANDLE Duplicate(void) const
	{
		return nullptr;
	}

	virtual ~CXCertStore(void)																			{	if (Handle()) ReleaseHandle();	}

	CXCertStore(void) : THandleHolder<HCERTSTORE, nullptr>()											{}
	CXCertStore(__in HCERTSTORE hStore) : THandleHolder<HCERTSTORE, nullptr>(hStore)					{}
};

#ifdef _UXTHEME_H_
class CXTheme : public THandleHolder<HTHEME, nullptr>
{
protected:
	virtual void ReleaseHandle(void)
	{
		::CloseThemeData(Handle());
	}
public:
	virtual HANDLE Duplicate(void) const
	{
		return nullptr;
	}

	virtual ~CXTheme(void)																				{	if (Handle()) ReleaseHandle();	}

	CXTheme(void) : THandleHolder<HTHEME, nullptr>()													{}
	CXTheme(__in HTHEME hTheme) : THandleHolder<HTHEME, nullptr>(hTheme)								{}
};
#endif

class CInterfaceMarker
{
private:
	CInterfaceMarker(void)														{}
	CInterfaceMarker(const CInterfaceMarker&)									{}
	CInterfaceMarker(CInterfaceMarker&&)										{}
	CInterfaceMarker& operator= (const CInterfaceMarker&)						{}
	CInterfaceMarker& operator= (CInterfaceMarker&&)							{}

	IUnknown*	mf_pUnk;
public:
	explicit CInterfaceMarker(IUnknown* pUnk) : mf_pUnk(pUnk)
	{
		if (pUnk)
			pUnk ->AddRef();
	}

	virtual ~CInterfaceMarker(void)
	{
		if (mf_pUnk)
			mf_pUnk ->Release();
	}
};

class CDPA
{
public:
	HDPA mf_hDPA;

	CDPA(void) : mf_hDPA(::DPA_Create(4))
	{
	}

	virtual ~CDPA(void)
	{
		if (mf_hDPA)
			::DPA_Destroy(mf_hDPA);
	}
};

template <typename _T = void*>
class CDSA
{
public:
	HDSA mf_HDSA;

	CDSA(__in int nDefaultItemsGrow = 4) :
		mf_HDSA(::DSA_Create(sizeof(_T), nDefaultItemsGrow))
	{}

	virtual ~CDSA(void)
	{
		Clear();
	}

	virtual void Clear(void)
	{
		if (mf_HDSA)
			::DSA_Destroy(mf_HDSA);
	}
};

class CCOMInitializer
{
private:
	CCOMInitializer(const CCOMInitializer&) {}
	CCOMInitializer(CCOMInitializer&&)		{}
	CCOMInitializer& operator= (const CCOMInitializer&) {	return *this;	}
	CCOMInitializer& operator= (CCOMInitializer&&)		{	return *this;	}

	HRESULT mf_hrInitializationResult;
	COINIT mf_ApartmentType;
public:
	CCOMInitializer(void)	
	{	
		mf_hrInitializationResult = ::CoInitialize(nullptr);
		mf_ApartmentType = COINIT_APARTMENTTHREADED;
	}

	explicit CCOMInitializer(__in COINIT ApartmentType)
	{
		mf_hrInitializationResult = ::CoInitializeEx(nullptr, ApartmentType);
		mf_ApartmentType = ApartmentType;
	}

	HRESULT InitializationResult(void)
	{
		return mf_hrInitializationResult;
	}

	virtual ~CCOMInitializer(void)
	{
		if (SUCCEEDED(InitializationResult()))
			::CoUninitialize();
	}
};

class CResString
{
private:
	CResString()	{}
protected:
	static BSTR LoadStringEx(__in HINSTANCE hInstance, __in DWORD MessageID, __in LANGID LangID)
	{
		BSTR lpStr(nullptr);
		HRSRC hResource(::FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(MessageID / 16 + 1), LangID));
		if (hResource) 
		{
			HGLOBAL hGlobal(::LoadResource(hInstance, hResource));
			if (hGlobal) 
			{
				const __wchar_t* pwszRes(reinterpret_cast<const __wchar_t*>(::LockResource(hGlobal)));
				if (pwszRes) 
				{
					for (DWORD i = 0; i < (MessageID & 15); i++) 
						pwszRes += 1 + *(WORD*)(pwszRes);

					UnlockResource(pwszRes);

					BSTR pwszStr(::SysAllocStringLen(nullptr, *reinterpret_cast<WORD*>(const_cast<__wchar_t*>(pwszRes))));

					if (pwszStr != nullptr) 
					{
						pwszStr[*(WORD*)(pwszRes)] = L'\0';
						::CopyMemory(pwszStr, pwszRes + 1, *reinterpret_cast<WORD*>(const_cast<__wchar_t*>(pwszRes)) * sizeof(__wchar_t));
					}
					lpStr = pwszStr;
				}
				::FreeResource(hGlobal);
			}
		} 
		return lpStr;
	}
public:
	ATL::CComBSTR m_bszResString;
	CResString& operator= (CResString&& Src)
	{
		if (&Src != this)
			m_bszResString.Attach(Src.m_bszResString.Detach());
		return (*this);
	}

	CResString& operator= (const CResString& Src)	
	{
		if (&Src != this)
			m_bszResString.Attach(Src.m_bszResString.Copy());
		return (*this);
	}

	void Clear(void)
	{
		if (m_bszResString.m_str)
			m_bszResString.Empty();
	}

	explicit CResString(__in DWORD dwMsgID) : m_bszResString(LoadStringEx(hDLLInstance, dwMsgID, ::GetSystemDefaultLangID()))
	{
		if (!m_bszResString.m_str)
			m_bszResString.m_str = LoadStringEx(hDLLInstance, dwMsgID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
	}

	virtual ~CResString(void)
	{
		Clear();
	}

	size_t Length(void) const
	{
		return m_bszResString.Length();
	}
};
