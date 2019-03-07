

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IPE32Viewer,0xC4CFC677,0x4ACB,0x4467,0xAD,0xA9,0xB2,0xB5,0xA5,0x47,0x2B,0x13);


MIDL_DEFINE_GUID(IID, IID_IPE32ImportsCallback,0x34155BED,0xA96B,0x45A4,0xA4,0xE5,0x2C,0x51,0x7A,0x05,0x26,0x78);


MIDL_DEFINE_GUID(IID, IID_IPE32ExportsCallback,0x8894E1F7,0x28F3,0x43AA,0x81,0x4E,0xD9,0xF9,0x81,0x48,0x44,0xDF);


MIDL_DEFINE_GUID(IID, IID_IPE32CertificatesCallback,0xDA256CF7,0xCBB1,0x4D71,0x89,0x06,0x1C,0xCB,0x06,0xB1,0xA1,0xAC);


MIDL_DEFINE_GUID(IID, IID_IPE32ResourcesCallback,0x4A893140,0x624A,0x438A,0x9F,0x22,0x69,0x7D,0x6D,0x91,0xF2,0xFC);


MIDL_DEFINE_GUID(IID, IID_IPE32StringsCallback,0xE7EA88AB,0x250B,0x4265,0x80,0x4F,0xBA,0xF1,0x11,0x2E,0xCB,0xED);


MIDL_DEFINE_GUID(IID, IID_IPE32MessagesCallback,0x05DA8F49,0xD036,0x4CD2,0x8A,0xBA,0x46,0x22,0x3A,0x26,0xBB,0xF5);


MIDL_DEFINE_GUID(IID, IID_IPE32TypelibCallback,0x9733C31C,0xCEE7,0x41F0,0xA5,0x6F,0xFD,0xAE,0x94,0xBE,0x47,0xD9);


MIDL_DEFINE_GUID(IID, IID_IPE32VersionCallback,0xE14716C2,0x7086,0x49DB,0x8D,0xB8,0x95,0xA5,0xBD,0xF8,0x34,0x72);


MIDL_DEFINE_GUID(IID, LIBID_PE32ViewerLib,0x4C0A29AF,0x0961,0x45E0,0xA2,0x8B,0x3D,0x73,0x47,0x68,0x71,0x46);


MIDL_DEFINE_GUID(CLSID, CLSID_CoPE32Viewer,0x3DD4738C,0xDFDB,0x4C82,0xB9,0x52,0xA7,0x98,0x5E,0xC5,0xF4,0xF7);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



