

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sun Feb 10 02:35:16 2019
 */
/* Compiler settings for PE32Viewer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __PE32Viewer_i_h__
#define __PE32Viewer_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPE32Viewer_FWD_DEFINED__
#define __IPE32Viewer_FWD_DEFINED__
typedef interface IPE32Viewer IPE32Viewer;
#endif 	/* __IPE32Viewer_FWD_DEFINED__ */


#ifndef __IPE32ImportsCallback_FWD_DEFINED__
#define __IPE32ImportsCallback_FWD_DEFINED__
typedef interface IPE32ImportsCallback IPE32ImportsCallback;
#endif 	/* __IPE32ImportsCallback_FWD_DEFINED__ */


#ifndef __IPE32ExportsCallback_FWD_DEFINED__
#define __IPE32ExportsCallback_FWD_DEFINED__
typedef interface IPE32ExportsCallback IPE32ExportsCallback;
#endif 	/* __IPE32ExportsCallback_FWD_DEFINED__ */


#ifndef __IPE32CertificatesCallback_FWD_DEFINED__
#define __IPE32CertificatesCallback_FWD_DEFINED__
typedef interface IPE32CertificatesCallback IPE32CertificatesCallback;
#endif 	/* __IPE32CertificatesCallback_FWD_DEFINED__ */


#ifndef __IPE32ResourcesCallback_FWD_DEFINED__
#define __IPE32ResourcesCallback_FWD_DEFINED__
typedef interface IPE32ResourcesCallback IPE32ResourcesCallback;
#endif 	/* __IPE32ResourcesCallback_FWD_DEFINED__ */


#ifndef __IPE32StringsCallback_FWD_DEFINED__
#define __IPE32StringsCallback_FWD_DEFINED__
typedef interface IPE32StringsCallback IPE32StringsCallback;
#endif 	/* __IPE32StringsCallback_FWD_DEFINED__ */


#ifndef __IPE32MessagesCallback_FWD_DEFINED__
#define __IPE32MessagesCallback_FWD_DEFINED__
typedef interface IPE32MessagesCallback IPE32MessagesCallback;
#endif 	/* __IPE32MessagesCallback_FWD_DEFINED__ */


#ifndef __IPE32TypelibCallback_FWD_DEFINED__
#define __IPE32TypelibCallback_FWD_DEFINED__
typedef interface IPE32TypelibCallback IPE32TypelibCallback;
#endif 	/* __IPE32TypelibCallback_FWD_DEFINED__ */


#ifndef __IPE32VersionCallback_FWD_DEFINED__
#define __IPE32VersionCallback_FWD_DEFINED__
typedef interface IPE32VersionCallback IPE32VersionCallback;
#endif 	/* __IPE32VersionCallback_FWD_DEFINED__ */


#ifndef __CoPE32Viewer_FWD_DEFINED__
#define __CoPE32Viewer_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoPE32Viewer CoPE32Viewer;
#else
typedef struct CoPE32Viewer CoPE32Viewer;
#endif /* __cplusplus */

#endif 	/* __CoPE32Viewer_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_PE32Viewer_0000_0000 */
/* [local] */ 

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("D10C7371-9D22-471E-A0EC-B5CA4B88F908") struct PE32VIEW_VERSION
    {
    unsigned long dwVerMajor;
    unsigned long dwVerMinor;
    unsigned long dwVerBuildNo;
    unsigned long dwVerSpecialNo;
    } 	PE32VIEW_VERSION;



extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0000_v0_0_s_ifspec;

#ifndef __IPE32Viewer_INTERFACE_DEFINED__
#define __IPE32Viewer_INTERFACE_DEFINED__

/* interface IPE32Viewer */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32Viewer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C4CFC677-4ACB-4467-ADA9-B2B5A5472B13")
    IPE32Viewer : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetVersion( 
            /* [retval][out] */ LONG *plVersion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetVersionEx( 
            /* [retval][out] */ PE32VIEW_VERSION *psVersionEx) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32ViewerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32Viewer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32Viewer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32Viewer * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IPE32Viewer * This,
            /* [retval][out] */ LONG *plVersion);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetVersionEx )( 
            IPE32Viewer * This,
            /* [retval][out] */ PE32VIEW_VERSION *psVersionEx);
        
        END_INTERFACE
    } IPE32ViewerVtbl;

    interface IPE32Viewer
    {
        CONST_VTBL struct IPE32ViewerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32Viewer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32Viewer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32Viewer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32Viewer_GetVersion(This,plVersion)	\
    ( (This)->lpVtbl -> GetVersion(This,plVersion) ) 

#define IPE32Viewer_GetVersionEx(This,psVersionEx)	\
    ( (This)->lpVtbl -> GetVersionEx(This,psVersionEx) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32Viewer_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_PE32Viewer_0000_0001 */
/* [local] */ 

typedef /* [helpstring][v1_enum][uuid] */  DECLSPEC_UUID("9E051683-F27F-4282-A422-1B58F25C5686") 
enum __tagPE32BOUND_STATE
    {	bsBounded	= -1,
	bsUnbounded	= 0
    } 	PE32BOUND_STATE;

typedef /* [helpstring][v1_enum][uuid] */  DECLSPEC_UUID("AC8A8DC0-D9D4-490E-9333-B40F89A48A64") 
enum __tagPE32IMPORT_KIND
    {	ikByOrdinal	= -1,
	ikByName	= 0
    } 	PE32IMPORT_KIND;

typedef /* [helpstring][v1_enum][uuid] */  DECLSPEC_UUID("78914C50-3DB1-47C5-BCCB-CE89A006FE5A") 
enum __tagPE32IMPORT_FORWARD_STATE
    {	fsForwarded	= -1,
	fsNotForwarded	= 0
    } 	PE32IMPORT_FORWARD_STATE;

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("F34DBE54-87B6-4021-A4E8-0FC8CE3C3001") struct __PEIMGIMPORTITEM
    {
    unsigned long dwStructSize;
    PE32BOUND_STATE BoundState;
    PE32IMPORT_KIND ImportKind;
    unsigned char *pAnsiName;
    unsigned long dwOrdinal;
    PE32IMPORT_FORWARD_STATE ForwardState;
    short wHint;
    } 	__PEIMGIMPORTITEM;

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("D31017B0-1FAF-468F-832D-31F6711B9326") struct __PEIMGEXPORTITEM
    {
    unsigned long dwStructSize;
    unsigned char *pAnsiName;
    unsigned long dwOrdinal;
    PE32IMPORT_FORWARD_STATE ForwardState;
    unsigned char *pForwardedName;
    } 	__PEIMGEXPORTITEM;

typedef /* [helpstring][v1_enum][uuid] */  DECLSPEC_UUID("C4E63320-C535-4C16-A87D-C0373D188F79") 
enum __tagPE32RES_ID_TYPE
    {	rtByString	= -1,
	fsByNum	= 0
    } 	PE32RES_ID_TYPE;

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("5EF101AA-25F3-4186-991A-AE0AB2E985A7") struct __PEIMGRESOURCEITEM
    {
    unsigned long dwStructSize;
    PE32RES_ID_TYPE ResIDType;
    unsigned long dwID;
    BSTR pwszName;
    LCID lcid;
    unsigned long dwCodePage;
    unsigned long dwResItemDataSize;
    byte *lpResItemDataPtr;
    } 	__PEIMGRESOURCEITEM;

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("2508933B-62E5-45C8-B597-A09742B19D2F") struct __PEIMGRESTYPELIBITEM
    {
    unsigned long dwStructSize;
    BSTR bstrShortName;
    BSTR bstrFullName;
    unsigned long ulReserved;
    } 	__PEIMGRESTYPELIBITEM;



extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0001_v0_0_s_ifspec;

#ifndef __IPE32ImportsCallback_INTERFACE_DEFINED__
#define __IPE32ImportsCallback_INTERFACE_DEFINED__

/* interface IPE32ImportsCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32ImportsCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34155BED-A96B-45A4-A4E5-2C517A052678")
    IPE32ImportsCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32ImportSymbols( 
            /* [in] */ BSTR pwszModuleName,
            /* [in] */ __PEIMGIMPORTITEM *pItemDesc,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32ImportsCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32ImportsCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32ImportsCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32ImportsCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32ImportSymbols )( 
            IPE32ImportsCallback * This,
            /* [in] */ BSTR pwszModuleName,
            /* [in] */ __PEIMGIMPORTITEM *pItemDesc,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32ImportsCallbackVtbl;

    interface IPE32ImportsCallback
    {
        CONST_VTBL struct IPE32ImportsCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32ImportsCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32ImportsCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32ImportsCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32ImportsCallback_EnumPE32ImportSymbols(This,pwszModuleName,pItemDesc,pContext)	\
    ( (This)->lpVtbl -> EnumPE32ImportSymbols(This,pwszModuleName,pItemDesc,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32ImportsCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32ExportsCallback_INTERFACE_DEFINED__
#define __IPE32ExportsCallback_INTERFACE_DEFINED__

/* interface IPE32ExportsCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32ExportsCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8894E1F7-28F3-43AA-814E-D9F9814844DF")
    IPE32ExportsCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32ExportSymbols( 
            /* [in] */ unsigned long nIndex,
            /* [in] */ __PEIMGEXPORTITEM *pItemDesc,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32ExportsCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32ExportsCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32ExportsCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32ExportsCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32ExportSymbols )( 
            IPE32ExportsCallback * This,
            /* [in] */ unsigned long nIndex,
            /* [in] */ __PEIMGEXPORTITEM *pItemDesc,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32ExportsCallbackVtbl;

    interface IPE32ExportsCallback
    {
        CONST_VTBL struct IPE32ExportsCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32ExportsCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32ExportsCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32ExportsCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32ExportsCallback_EnumPE32ExportSymbols(This,nIndex,pItemDesc,pContext)	\
    ( (This)->lpVtbl -> EnumPE32ExportSymbols(This,nIndex,pItemDesc,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32ExportsCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32CertificatesCallback_INTERFACE_DEFINED__
#define __IPE32CertificatesCallback_INTERFACE_DEFINED__

/* interface IPE32CertificatesCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32CertificatesCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DA256CF7-CBB1-4D71-8906-1CCB06B1A1AC")
    IPE32CertificatesCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32Certificates( 
            /* [in] */ unsigned long dwIndex,
            /* [in] */ BSTR pwszCertSubject,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32CertificatesCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32CertificatesCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32CertificatesCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32CertificatesCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32Certificates )( 
            IPE32CertificatesCallback * This,
            /* [in] */ unsigned long dwIndex,
            /* [in] */ BSTR pwszCertSubject,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32CertificatesCallbackVtbl;

    interface IPE32CertificatesCallback
    {
        CONST_VTBL struct IPE32CertificatesCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32CertificatesCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32CertificatesCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32CertificatesCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32CertificatesCallback_EnumPE32Certificates(This,dwIndex,pwszCertSubject,pContext)	\
    ( (This)->lpVtbl -> EnumPE32Certificates(This,dwIndex,pwszCertSubject,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32CertificatesCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32ResourcesCallback_INTERFACE_DEFINED__
#define __IPE32ResourcesCallback_INTERFACE_DEFINED__

/* interface IPE32ResourcesCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32ResourcesCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4A893140-624A-438A-9F22-697D6D91F2FC")
    IPE32ResourcesCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32ResourcesTypes( 
            /* [in] */ unsigned long dwResType,
            /* [in] */ BSTR bstrResTypeName,
            /* [in] */ byte *pContext) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32ResourcesByType( 
            /* [in] */ unsigned long dwResType,
            /* [in] */ __PEIMGRESOURCEITEM *pResItemDesc,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32ResourcesCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32ResourcesCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32ResourcesCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32ResourcesCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32ResourcesTypes )( 
            IPE32ResourcesCallback * This,
            /* [in] */ unsigned long dwResType,
            /* [in] */ BSTR bstrResTypeName,
            /* [in] */ byte *pContext);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32ResourcesByType )( 
            IPE32ResourcesCallback * This,
            /* [in] */ unsigned long dwResType,
            /* [in] */ __PEIMGRESOURCEITEM *pResItemDesc,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32ResourcesCallbackVtbl;

    interface IPE32ResourcesCallback
    {
        CONST_VTBL struct IPE32ResourcesCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32ResourcesCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32ResourcesCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32ResourcesCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32ResourcesCallback_EnumPE32ResourcesTypes(This,dwResType,bstrResTypeName,pContext)	\
    ( (This)->lpVtbl -> EnumPE32ResourcesTypes(This,dwResType,bstrResTypeName,pContext) ) 

#define IPE32ResourcesCallback_EnumPE32ResourcesByType(This,dwResType,pResItemDesc,pContext)	\
    ( (This)->lpVtbl -> EnumPE32ResourcesByType(This,dwResType,pResItemDesc,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32ResourcesCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32StringsCallback_INTERFACE_DEFINED__
#define __IPE32StringsCallback_INTERFACE_DEFINED__

/* interface IPE32StringsCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32StringsCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E7EA88AB-250B-4265-804F-BAF1112ECBED")
    IPE32StringsCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32StringResources( 
            /* [in] */ LCID lcid,
            /* [in] */ DWORD dwID,
            /* [in] */ byte *pResData,
            /* [in] */ DWORD dwStringLen,
            /* [in] */ BSTR pwszString,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32StringsCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32StringsCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32StringsCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32StringsCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32StringResources )( 
            IPE32StringsCallback * This,
            /* [in] */ LCID lcid,
            /* [in] */ DWORD dwID,
            /* [in] */ byte *pResData,
            /* [in] */ DWORD dwStringLen,
            /* [in] */ BSTR pwszString,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32StringsCallbackVtbl;

    interface IPE32StringsCallback
    {
        CONST_VTBL struct IPE32StringsCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32StringsCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32StringsCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32StringsCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32StringsCallback_EnumPE32StringResources(This,lcid,dwID,pResData,dwStringLen,pwszString,pContext)	\
    ( (This)->lpVtbl -> EnumPE32StringResources(This,lcid,dwID,pResData,dwStringLen,pwszString,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32StringsCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32MessagesCallback_INTERFACE_DEFINED__
#define __IPE32MessagesCallback_INTERFACE_DEFINED__

/* interface IPE32MessagesCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32MessagesCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("05DA8F49-D036-4CD2-8ABA-46223A26BBF5")
    IPE32MessagesCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32MsgTableItemsResources( 
            /* [in] */ LCID lcid,
            /* [in] */ DWORD dwMsgID,
            /* [in] */ byte *pResData,
            /* [in] */ DWORD dwID,
            /* [in] */ DWORD dwStringLen,
            /* [in] */ BSTR pwszString,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32MessagesCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32MessagesCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32MessagesCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32MessagesCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32MsgTableItemsResources )( 
            IPE32MessagesCallback * This,
            /* [in] */ LCID lcid,
            /* [in] */ DWORD dwMsgID,
            /* [in] */ byte *pResData,
            /* [in] */ DWORD dwID,
            /* [in] */ DWORD dwStringLen,
            /* [in] */ BSTR pwszString,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32MessagesCallbackVtbl;

    interface IPE32MessagesCallback
    {
        CONST_VTBL struct IPE32MessagesCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32MessagesCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32MessagesCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32MessagesCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32MessagesCallback_EnumPE32MsgTableItemsResources(This,lcid,dwMsgID,pResData,dwID,dwStringLen,pwszString,pContext)	\
    ( (This)->lpVtbl -> EnumPE32MsgTableItemsResources(This,lcid,dwMsgID,pResData,dwID,dwStringLen,pwszString,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32MessagesCallback_INTERFACE_DEFINED__ */


#ifndef __IPE32TypelibCallback_INTERFACE_DEFINED__
#define __IPE32TypelibCallback_INTERFACE_DEFINED__

/* interface IPE32TypelibCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32TypelibCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9733C31C-CEE7-41F0-A56F-FDAE94BE47D9")
    IPE32TypelibCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32Typelib( 
            /* [in] */ __PEIMGRESTYPELIBITEM *pTypelibDesc,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32TypelibCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32TypelibCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32TypelibCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32TypelibCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32Typelib )( 
            IPE32TypelibCallback * This,
            /* [in] */ __PEIMGRESTYPELIBITEM *pTypelibDesc,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32TypelibCallbackVtbl;

    interface IPE32TypelibCallback
    {
        CONST_VTBL struct IPE32TypelibCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32TypelibCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32TypelibCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32TypelibCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32TypelibCallback_EnumPE32Typelib(This,pTypelibDesc,pContext)	\
    ( (This)->lpVtbl -> EnumPE32Typelib(This,pTypelibDesc,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32TypelibCallback_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_PE32Viewer_0000_0008 */
/* [local] */ 

typedef /* [helpstring][version][uuid] */  DECLSPEC_UUID("E050ED46-94D2-40D5-BA49-AA8F4D8CDBA4") struct LANGANDCODEPAGE
    {
    short wLanguage;
    short wCodePage;
    } 	LANGANDCODEPAGE;



extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0008_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_PE32Viewer_0000_0008_v0_0_s_ifspec;

#ifndef __IPE32VersionCallback_INTERFACE_DEFINED__
#define __IPE32VersionCallback_INTERFACE_DEFINED__

/* interface IPE32VersionCallback */
/* [helpstring][unique][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IPE32VersionCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E14716C2-7086-49DB-8DB8-95A5BDF83472")
    IPE32VersionCallback : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumPE32VersionStringTableStrings( 
            /* [in] */ unsigned short wLanguage,
            /* [in] */ unsigned short wCodePage,
            /* [in] */ BSTR bstrKey,
            /* [in] */ BSTR bstrValue,
            /* [in] */ byte *pContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPE32VersionCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPE32VersionCallback * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPE32VersionCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPE32VersionCallback * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumPE32VersionStringTableStrings )( 
            IPE32VersionCallback * This,
            /* [in] */ unsigned short wLanguage,
            /* [in] */ unsigned short wCodePage,
            /* [in] */ BSTR bstrKey,
            /* [in] */ BSTR bstrValue,
            /* [in] */ byte *pContext);
        
        END_INTERFACE
    } IPE32VersionCallbackVtbl;

    interface IPE32VersionCallback
    {
        CONST_VTBL struct IPE32VersionCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPE32VersionCallback_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPE32VersionCallback_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPE32VersionCallback_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPE32VersionCallback_EnumPE32VersionStringTableStrings(This,wLanguage,wCodePage,bstrKey,bstrValue,pContext)	\
    ( (This)->lpVtbl -> EnumPE32VersionStringTableStrings(This,wLanguage,wCodePage,bstrKey,bstrValue,pContext) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPE32VersionCallback_INTERFACE_DEFINED__ */



#ifndef __PE32ViewerLib_LIBRARY_DEFINED__
#define __PE32ViewerLib_LIBRARY_DEFINED__

/* library PE32ViewerLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_PE32ViewerLib;

EXTERN_C const CLSID CLSID_CoPE32Viewer;

#ifdef __cplusplus

class DECLSPEC_UUID("3DD4738C-DFDB-4C82-B952-A7985EC5F4F7")
CoPE32Viewer;
#endif
#endif /* __PE32ViewerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


