// dllmain.h : Declaration of module class.

class CPE32ViewerModule : public ATL::CAtlDllModuleT< CPE32ViewerModule >
{
public :
	DECLARE_LIBID(LIBID_PE32ViewerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PE32VIEWER, "{4752E8BD-7EEF-4F04-9127-1ABF72DCF39C}")
};

extern class CPE32ViewerModule _AtlModule;
