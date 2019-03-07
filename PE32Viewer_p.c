

/* this ALWAYS GENERATED file contains the proxy stub code */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Thu Mar 07 00:02:26 2019
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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "PE32Viewer_i.h"

#define TYPE_FORMAT_STRING_SIZE   177                               
#define PROC_FORMAT_STRING_SIZE   505                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _PE32Viewer_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } PE32Viewer_MIDL_TYPE_FORMAT_STRING;

typedef struct _PE32Viewer_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } PE32Viewer_MIDL_PROC_FORMAT_STRING;

typedef struct _PE32Viewer_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } PE32Viewer_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const PE32Viewer_MIDL_TYPE_FORMAT_STRING PE32Viewer__MIDL_TypeFormatString;
extern const PE32Viewer_MIDL_PROC_FORMAT_STRING PE32Viewer__MIDL_ProcFormatString;
extern const PE32Viewer_MIDL_EXPR_FORMAT_STRING PE32Viewer__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32Viewer_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32Viewer_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32ImportsCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32ImportsCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32ExportsCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32ExportsCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32CertificatesCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32CertificatesCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32ResourcesCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32ResourcesCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32StringsCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32StringsCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32MessagesCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32MessagesCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32TypelibCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32TypelibCallback_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPE32VersionCallback_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IPE32VersionCallback_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const PE32Viewer_MIDL_PROC_FORMAT_STRING PE32Viewer__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure GetVersion */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x3 ),	/* 3 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x24 ),	/* 36 */
/* 14 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 16 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x0 ),	/* 0 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter plVersion */

/* 24 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 26 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 28 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 30 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 32 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 34 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetVersionEx */

/* 36 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 38 */	NdrFcLong( 0x0 ),	/* 0 */
/* 42 */	NdrFcShort( 0x4 ),	/* 4 */
/* 44 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 46 */	NdrFcShort( 0x0 ),	/* 0 */
/* 48 */	NdrFcShort( 0x3c ),	/* 60 */
/* 50 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 52 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 54 */	NdrFcShort( 0x0 ),	/* 0 */
/* 56 */	NdrFcShort( 0x0 ),	/* 0 */
/* 58 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter psVersionEx */

/* 60 */	NdrFcShort( 0x4112 ),	/* Flags:  must free, out, simple ref, srv alloc size=16 */
/* 62 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 64 */	NdrFcShort( 0xa ),	/* Type Offset=10 */

	/* Return value */

/* 66 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 68 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 70 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32ImportSymbols */

/* 72 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 74 */	NdrFcLong( 0x0 ),	/* 0 */
/* 78 */	NdrFcShort( 0x3 ),	/* 3 */
/* 80 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 82 */	NdrFcShort( 0x19 ),	/* 25 */
/* 84 */	NdrFcShort( 0x8 ),	/* 8 */
/* 86 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x4,		/* 4 */
/* 88 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 90 */	NdrFcShort( 0x0 ),	/* 0 */
/* 92 */	NdrFcShort( 0x1 ),	/* 1 */
/* 94 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pwszModuleName */

/* 96 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 98 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 100 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter pItemDesc */

/* 102 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 104 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 106 */	NdrFcShort( 0x3c ),	/* Type Offset=60 */

	/* Parameter pContext */

/* 108 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 110 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 112 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 114 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 116 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 118 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32ExportSymbols */

/* 120 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 122 */	NdrFcLong( 0x0 ),	/* 0 */
/* 126 */	NdrFcShort( 0x3 ),	/* 3 */
/* 128 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 130 */	NdrFcShort( 0x83 ),	/* 131 */
/* 132 */	NdrFcShort( 0x8 ),	/* 8 */
/* 134 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x4,		/* 4 */
/* 136 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x0 ),	/* 0 */
/* 142 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter nIndex */

/* 144 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 146 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 148 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pItemDesc */

/* 150 */	NdrFcShort( 0x10a ),	/* Flags:  must free, in, simple ref, */
/* 152 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 154 */	NdrFcShort( 0x5a ),	/* Type Offset=90 */

	/* Parameter pContext */

/* 156 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 158 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 160 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 162 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 164 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 166 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32ResourcesTypes */


	/* Procedure EnumPE32Certificates */

/* 168 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 170 */	NdrFcLong( 0x0 ),	/* 0 */
/* 174 */	NdrFcShort( 0x3 ),	/* 3 */
/* 176 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 178 */	NdrFcShort( 0x21 ),	/* 33 */
/* 180 */	NdrFcShort( 0x8 ),	/* 8 */
/* 182 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x4,		/* 4 */
/* 184 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 188 */	NdrFcShort( 0x1 ),	/* 1 */
/* 190 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter dwResType */


	/* Parameter dwIndex */

/* 192 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 194 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 196 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter bstrResTypeName */


	/* Parameter pwszCertSubject */

/* 198 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 200 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 202 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter pContext */


	/* Parameter pContext */

/* 204 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 206 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 208 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */


	/* Return value */

/* 210 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 212 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 214 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32ResourcesByType */

/* 216 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 218 */	NdrFcLong( 0x0 ),	/* 0 */
/* 222 */	NdrFcShort( 0x4 ),	/* 4 */
/* 224 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 226 */	NdrFcShort( 0x21 ),	/* 33 */
/* 228 */	NdrFcShort( 0x8 ),	/* 8 */
/* 230 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x4,		/* 4 */
/* 232 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 234 */	NdrFcShort( 0x0 ),	/* 0 */
/* 236 */	NdrFcShort( 0x1 ),	/* 1 */
/* 238 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter dwResType */

/* 240 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 242 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 244 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pResItemDesc */

/* 246 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 248 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 250 */	NdrFcShort( 0x80 ),	/* Type Offset=128 */

	/* Parameter pContext */

/* 252 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 254 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 256 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 258 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 260 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 262 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32StringResources */

/* 264 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 266 */	NdrFcLong( 0x0 ),	/* 0 */
/* 270 */	NdrFcShort( 0x3 ),	/* 3 */
/* 272 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 274 */	NdrFcShort( 0x4a ),	/* 74 */
/* 276 */	NdrFcShort( 0x8 ),	/* 8 */
/* 278 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x7,		/* 7 */
/* 280 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 282 */	NdrFcShort( 0x0 ),	/* 0 */
/* 284 */	NdrFcShort( 0x1 ),	/* 1 */
/* 286 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter lcid */

/* 288 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 290 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 292 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwID */

/* 294 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 296 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 298 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pResData */

/* 300 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 302 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 304 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Parameter dwStringLen */

/* 306 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 308 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 310 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pwszString */

/* 312 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 314 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 316 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter pContext */

/* 318 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 320 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 322 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 324 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 326 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 328 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32MsgTableItemsResources */

/* 330 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 332 */	NdrFcLong( 0x0 ),	/* 0 */
/* 336 */	NdrFcShort( 0x3 ),	/* 3 */
/* 338 */	NdrFcShort( 0x24 ),	/* x86 Stack size/offset = 36 */
/* 340 */	NdrFcShort( 0x52 ),	/* 82 */
/* 342 */	NdrFcShort( 0x8 ),	/* 8 */
/* 344 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x8,		/* 8 */
/* 346 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 350 */	NdrFcShort( 0x1 ),	/* 1 */
/* 352 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter lcid */

/* 354 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 356 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 358 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwMsgID */

/* 360 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 362 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 364 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pResData */

/* 366 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 368 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 370 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Parameter dwID */

/* 372 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 374 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 376 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwStringLen */

/* 378 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 380 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 382 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pwszString */

/* 384 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 386 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 388 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter pContext */

/* 390 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 392 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 394 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 396 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 398 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 400 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32Typelib */

/* 402 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 404 */	NdrFcLong( 0x0 ),	/* 0 */
/* 408 */	NdrFcShort( 0x3 ),	/* 3 */
/* 410 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 412 */	NdrFcShort( 0x19 ),	/* 25 */
/* 414 */	NdrFcShort( 0x8 ),	/* 8 */
/* 416 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 418 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 420 */	NdrFcShort( 0x0 ),	/* 0 */
/* 422 */	NdrFcShort( 0x1 ),	/* 1 */
/* 424 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter pTypelibDesc */

/* 426 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 428 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 430 */	NdrFcShort( 0x9c ),	/* Type Offset=156 */

	/* Parameter pContext */

/* 432 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 434 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 436 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 438 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 440 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 442 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure EnumPE32VersionStringTableStrings */

/* 444 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 446 */	NdrFcLong( 0x0 ),	/* 0 */
/* 450 */	NdrFcShort( 0x3 ),	/* 3 */
/* 452 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 454 */	NdrFcShort( 0x25 ),	/* 37 */
/* 456 */	NdrFcShort( 0x8 ),	/* 8 */
/* 458 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x6,		/* 6 */
/* 460 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 462 */	NdrFcShort( 0x0 ),	/* 0 */
/* 464 */	NdrFcShort( 0x1 ),	/* 1 */
/* 466 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter wLanguage */

/* 468 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 470 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 472 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Parameter wCodePage */

/* 474 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 476 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 478 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Parameter bstrKey */

/* 480 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 482 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 484 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter bstrValue */

/* 486 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 488 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 490 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter pContext */

/* 492 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 494 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 496 */	0x1,		/* FC_BYTE */
			0x0,		/* 0 */

	/* Return value */

/* 498 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 500 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 502 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const PE32Viewer_MIDL_TYPE_FORMAT_STRING PE32Viewer__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/*  4 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/*  6 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/*  8 */	NdrFcShort( 0x2 ),	/* Offset= 2 (10) */
/* 10 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 12 */	NdrFcShort( 0x10 ),	/* 16 */
/* 14 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 16 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 18 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 20 */	
			0x12, 0x0,	/* FC_UP */
/* 22 */	NdrFcShort( 0xe ),	/* Offset= 14 (36) */
/* 24 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 26 */	NdrFcShort( 0x2 ),	/* 2 */
/* 28 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 30 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 32 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 34 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 36 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 38 */	NdrFcShort( 0x8 ),	/* 8 */
/* 40 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (24) */
/* 42 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 44 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 46 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 48 */	NdrFcShort( 0x0 ),	/* 0 */
/* 50 */	NdrFcShort( 0x4 ),	/* 4 */
/* 52 */	NdrFcShort( 0x0 ),	/* 0 */
/* 54 */	NdrFcShort( 0xffde ),	/* Offset= -34 (20) */
/* 56 */	
			0x11, 0x0,	/* FC_RP */
/* 58 */	NdrFcShort( 0x2 ),	/* Offset= 2 (60) */
/* 60 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 62 */	NdrFcShort( 0x1c ),	/* 28 */
/* 64 */	NdrFcShort( 0x0 ),	/* 0 */
/* 66 */	NdrFcShort( 0xc ),	/* Offset= 12 (78) */
/* 68 */	0x8,		/* FC_LONG */
			0xe,		/* FC_ENUM32 */
/* 70 */	0xe,		/* FC_ENUM32 */
			0x36,		/* FC_POINTER */
/* 72 */	0x8,		/* FC_LONG */
			0xe,		/* FC_ENUM32 */
/* 74 */	0x6,		/* FC_SHORT */
			0x3e,		/* FC_STRUCTPAD2 */
/* 76 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 78 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 80 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 82 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 84 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 86 */	
			0x11, 0x0,	/* FC_RP */
/* 88 */	NdrFcShort( 0x2 ),	/* Offset= 2 (90) */
/* 90 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 92 */	NdrFcShort( 0x14 ),	/* 20 */
/* 94 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 96 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 98 */	NdrFcShort( 0x4 ),	/* 4 */
/* 100 */	NdrFcShort( 0x4 ),	/* 4 */
/* 102 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 104 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 106 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 108 */	NdrFcShort( 0x10 ),	/* 16 */
/* 110 */	NdrFcShort( 0x10 ),	/* 16 */
/* 112 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 114 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 116 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 118 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 120 */	0xe,		/* FC_ENUM32 */
			0x8,		/* FC_LONG */
/* 122 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 124 */	
			0x11, 0x0,	/* FC_RP */
/* 126 */	NdrFcShort( 0x2 ),	/* Offset= 2 (128) */
/* 128 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 130 */	NdrFcShort( 0x20 ),	/* 32 */
/* 132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 134 */	NdrFcShort( 0xe ),	/* Offset= 14 (148) */
/* 136 */	0x8,		/* FC_LONG */
			0xe,		/* FC_ENUM32 */
/* 138 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 140 */	0x0,		/* 0 */
			NdrFcShort( 0xffa1 ),	/* Offset= -95 (46) */
			0x8,		/* FC_LONG */
/* 144 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 146 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 148 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 150 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 152 */	
			0x11, 0x0,	/* FC_RP */
/* 154 */	NdrFcShort( 0x2 ),	/* Offset= 2 (156) */
/* 156 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 158 */	NdrFcShort( 0x10 ),	/* 16 */
/* 160 */	NdrFcShort( 0x0 ),	/* 0 */
/* 162 */	NdrFcShort( 0x0 ),	/* Offset= 0 (162) */
/* 164 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 166 */	0x0,		/* 0 */
			NdrFcShort( 0xff87 ),	/* Offset= -121 (46) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 170 */	0x0,		/* 0 */
			NdrFcShort( 0xff83 ),	/* Offset= -125 (46) */
			0x8,		/* FC_LONG */
/* 174 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Standard interface: __MIDL_itf_PE32Viewer_0000_0000, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IPE32Viewer, ver. 0.0,
   GUID={0xC4CFC677,0x4ACB,0x4467,{0xAD,0xA9,0xB2,0xB5,0xA5,0x47,0x2B,0x13}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32Viewer_FormatStringOffsetTable[] =
    {
    0,
    36
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32Viewer_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32Viewer_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32Viewer_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32Viewer_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _IPE32ViewerProxyVtbl = 
{
    &IPE32Viewer_ProxyInfo,
    &IID_IPE32Viewer,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32Viewer::GetVersion */ ,
    (void *) (INT_PTR) -1 /* IPE32Viewer::GetVersionEx */
};

const CInterfaceStubVtbl _IPE32ViewerStubVtbl =
{
    &IID_IPE32Viewer,
    &IPE32Viewer_ServerInfo,
    5,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Standard interface: __MIDL_itf_PE32Viewer_0000_0001, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IPE32ImportsCallback, ver. 0.0,
   GUID={0x34155BED,0xA96B,0x45A4,{0xA4,0xE5,0x2C,0x51,0x7A,0x05,0x26,0x78}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32ImportsCallback_FormatStringOffsetTable[] =
    {
    72
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32ImportsCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ImportsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32ImportsCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ImportsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32ImportsCallbackProxyVtbl = 
{
    &IPE32ImportsCallback_ProxyInfo,
    &IID_IPE32ImportsCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32ImportsCallback::EnumPE32ImportSymbols */
};

const CInterfaceStubVtbl _IPE32ImportsCallbackStubVtbl =
{
    &IID_IPE32ImportsCallback,
    &IPE32ImportsCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32ExportsCallback, ver. 0.0,
   GUID={0x8894E1F7,0x28F3,0x43AA,{0x81,0x4E,0xD9,0xF9,0x81,0x48,0x44,0xDF}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32ExportsCallback_FormatStringOffsetTable[] =
    {
    120
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32ExportsCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ExportsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32ExportsCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ExportsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32ExportsCallbackProxyVtbl = 
{
    &IPE32ExportsCallback_ProxyInfo,
    &IID_IPE32ExportsCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32ExportsCallback::EnumPE32ExportSymbols */
};

const CInterfaceStubVtbl _IPE32ExportsCallbackStubVtbl =
{
    &IID_IPE32ExportsCallback,
    &IPE32ExportsCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32CertificatesCallback, ver. 0.0,
   GUID={0xDA256CF7,0xCBB1,0x4D71,{0x89,0x06,0x1C,0xCB,0x06,0xB1,0xA1,0xAC}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32CertificatesCallback_FormatStringOffsetTable[] =
    {
    168
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32CertificatesCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32CertificatesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32CertificatesCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32CertificatesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32CertificatesCallbackProxyVtbl = 
{
    &IPE32CertificatesCallback_ProxyInfo,
    &IID_IPE32CertificatesCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32CertificatesCallback::EnumPE32Certificates */
};

const CInterfaceStubVtbl _IPE32CertificatesCallbackStubVtbl =
{
    &IID_IPE32CertificatesCallback,
    &IPE32CertificatesCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32ResourcesCallback, ver. 0.0,
   GUID={0x4A893140,0x624A,0x438A,{0x9F,0x22,0x69,0x7D,0x6D,0x91,0xF2,0xFC}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32ResourcesCallback_FormatStringOffsetTable[] =
    {
    168,
    216
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32ResourcesCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ResourcesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32ResourcesCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32ResourcesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(5) _IPE32ResourcesCallbackProxyVtbl = 
{
    &IPE32ResourcesCallback_ProxyInfo,
    &IID_IPE32ResourcesCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32ResourcesCallback::EnumPE32ResourcesTypes */ ,
    (void *) (INT_PTR) -1 /* IPE32ResourcesCallback::EnumPE32ResourcesByType */
};

const CInterfaceStubVtbl _IPE32ResourcesCallbackStubVtbl =
{
    &IID_IPE32ResourcesCallback,
    &IPE32ResourcesCallback_ServerInfo,
    5,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32StringsCallback, ver. 0.0,
   GUID={0xE7EA88AB,0x250B,0x4265,{0x80,0x4F,0xBA,0xF1,0x11,0x2E,0xCB,0xED}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32StringsCallback_FormatStringOffsetTable[] =
    {
    264
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32StringsCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32StringsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32StringsCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32StringsCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32StringsCallbackProxyVtbl = 
{
    &IPE32StringsCallback_ProxyInfo,
    &IID_IPE32StringsCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32StringsCallback::EnumPE32StringResources */
};

const CInterfaceStubVtbl _IPE32StringsCallbackStubVtbl =
{
    &IID_IPE32StringsCallback,
    &IPE32StringsCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32MessagesCallback, ver. 0.0,
   GUID={0x05DA8F49,0xD036,0x4CD2,{0x8A,0xBA,0x46,0x22,0x3A,0x26,0xBB,0xF5}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32MessagesCallback_FormatStringOffsetTable[] =
    {
    330
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32MessagesCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32MessagesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32MessagesCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32MessagesCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32MessagesCallbackProxyVtbl = 
{
    &IPE32MessagesCallback_ProxyInfo,
    &IID_IPE32MessagesCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32MessagesCallback::EnumPE32MsgTableItemsResources */
};

const CInterfaceStubVtbl _IPE32MessagesCallbackStubVtbl =
{
    &IID_IPE32MessagesCallback,
    &IPE32MessagesCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPE32TypelibCallback, ver. 0.0,
   GUID={0x9733C31C,0xCEE7,0x41F0,{0xA5,0x6F,0xFD,0xAE,0x94,0xBE,0x47,0xD9}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32TypelibCallback_FormatStringOffsetTable[] =
    {
    402
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32TypelibCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32TypelibCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32TypelibCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32TypelibCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32TypelibCallbackProxyVtbl = 
{
    &IPE32TypelibCallback_ProxyInfo,
    &IID_IPE32TypelibCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32TypelibCallback::EnumPE32Typelib */
};

const CInterfaceStubVtbl _IPE32TypelibCallbackStubVtbl =
{
    &IID_IPE32TypelibCallback,
    &IPE32TypelibCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Standard interface: __MIDL_itf_PE32Viewer_0000_0008, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IPE32VersionCallback, ver. 0.0,
   GUID={0xE14716C2,0x7086,0x49DB,{0x8D,0xB8,0x95,0xA5,0xBD,0xF8,0x34,0x72}} */

#pragma code_seg(".orpc")
static const unsigned short IPE32VersionCallback_FormatStringOffsetTable[] =
    {
    444
    };

static const MIDL_STUBLESS_PROXY_INFO IPE32VersionCallback_ProxyInfo =
    {
    &Object_StubDesc,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32VersionCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IPE32VersionCallback_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    PE32Viewer__MIDL_ProcFormatString.Format,
    &IPE32VersionCallback_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(4) _IPE32VersionCallbackProxyVtbl = 
{
    &IPE32VersionCallback_ProxyInfo,
    &IID_IPE32VersionCallback,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IPE32VersionCallback::EnumPE32VersionStringTableStrings */
};

const CInterfaceStubVtbl _IPE32VersionCallbackStubVtbl =
{
    &IID_IPE32VersionCallback,
    &IPE32VersionCallback_ServerInfo,
    4,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    PE32Viewer__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * const _PE32Viewer_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IPE32TypelibCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32ResourcesCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32MessagesCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32ViewerProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32StringsCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32VersionCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32ImportsCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32CertificatesCallbackProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPE32ExportsCallbackProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _PE32Viewer_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IPE32TypelibCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32ResourcesCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32MessagesCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32ViewerStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32StringsCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32VersionCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32ImportsCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32CertificatesCallbackStubVtbl,
    ( CInterfaceStubVtbl *) &_IPE32ExportsCallbackStubVtbl,
    0
};

PCInterfaceName const _PE32Viewer_InterfaceNamesList[] = 
{
    "IPE32TypelibCallback",
    "IPE32ResourcesCallback",
    "IPE32MessagesCallback",
    "IPE32Viewer",
    "IPE32StringsCallback",
    "IPE32VersionCallback",
    "IPE32ImportsCallback",
    "IPE32CertificatesCallback",
    "IPE32ExportsCallback",
    0
};


#define _PE32Viewer_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _PE32Viewer, pIID, n)

int __stdcall _PE32Viewer_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _PE32Viewer, 9, 8 )
    IID_BS_LOOKUP_NEXT_TEST( _PE32Viewer, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _PE32Viewer, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _PE32Viewer, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _PE32Viewer, 9, *pIndex )
    
}

const ExtendedProxyFileInfo PE32Viewer_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _PE32Viewer_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _PE32Viewer_StubVtblList,
    (const PCInterfaceName * ) & _PE32Viewer_InterfaceNamesList,
    0, /* no delegation */
    & _PE32Viewer_IID_Lookup, 
    9,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

