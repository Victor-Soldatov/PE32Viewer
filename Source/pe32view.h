//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+---------------------+-------------------------------+
//  |S|R|C|N|r|    Facility         |               Code            |
//  +-+-+-+-+-+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail (COERROR)
//
//      R - reserved portion of the facility code, corresponds to NT's
//              second severity bit.
//
//      C - reserved portion of the facility code, corresponds to NT's
//              C field.
//
//      N - reserved portion of the facility code. Used to indicate a
//              mapped NT status value.
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_PE32VIEW                0x37A


//
// Define the severity codes
//
#define STATUS_SEV_PE32VIEW_WARNING      0x2
#define STATUS_SEV_PE32VIEW_SUCCESS      0x0
#define STATUS_SEV_PE32VIEW_INFORMATIONAL 0x1
#define STATUS_SEV_PE32VIEW_ERROR        0x3


//
// MessageId: MSG_ERROR_XMM_SUCCESS
//
// MessageText:
//
// Success.
//
#define MSG_ERROR_XMM_SUCCESS            ((DWORD)0x037A0000L)

//
// MessageId: MSG_ERROR_PE32VIEW_UNINITIALIZED
//
// MessageText:
//
// Object is not initialized.
//
#define MSG_ERROR_PE32VIEW_UNINITIALIZED ((DWORD)0xC37A0001L)

//
// MessageId: MSG_ERROR_PE32VIEW_INITIALIZATION_FAILED
//
// MessageText:
//
// Object initialization is failed
//
#define MSG_ERROR_PE32VIEW_INITIALIZATION_FAILED ((DWORD)0xC37A0002L)

//
// MessageId: MSG_ERROR_PE32VIEW_UNEXPECTED_FAULT
//
// MessageText:
//
// Unexpected fault
//
#define MSG_ERROR_PE32VIEW_UNEXPECTED_FAULT ((DWORD)0xC37A0003L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_ARGUMENTS
//
// MessageText:
//
// Invalid arguments
//
#define MSG_ERROR_PE32VIEW_INVALID_ARGUMENTS ((DWORD)0xC37A0004L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_DOS_HEADER
//
// MessageText:
//
// Invalid DOS header
//
#define MSG_ERROR_PE32VIEW_INVALID_DOS_HEADER ((DWORD)0xC37A0005L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_NT_HEADER
//
// MessageText:
//
// Invalid NT header
//
#define MSG_ERROR_PE32VIEW_INVALID_NT_HEADER ((DWORD)0xC37A0006L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_OPT_HEADER
//
// MessageText:
//
// Invalid optional header
//
#define MSG_ERROR_PE32VIEW_INVALID_OPT_HEADER ((DWORD)0xC37A0007L)

//
// MessageId: MSG_ERROR_PE32VIEW_WINTRUST_FAILED
//
// MessageText:
//
// WinTrust is failed
//
#define MSG_ERROR_PE32VIEW_WINTRUST_FAILED ((DWORD)0xC37A0008L)

//
// MessageId: MSG_ERROR_PE32VIEW_IMALLOC_FAILED
//
// MessageText:
//
// IMalloc is failed
//
#define MSG_ERROR_PE32VIEW_IMALLOC_FAILED ((DWORD)0xC37A0009L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_CERT_DESC
//
// MessageText:
//
// No certificate description
//
#define MSG_ERROR_PE32VIEW_NO_CERT_DESC  ((DWORD)0xC37A000AL)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_CERT_SUBJ
//
// MessageText:
//
// No certificate subject
//
#define MSG_ERROR_PE32VIEW_NO_CERT_SUBJ  ((DWORD)0xC37A000BL)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_IMPORT
//
// MessageText:
//
// No import symbols
//
#define MSG_ERROR_PE32VIEW_NO_IMPORT     ((DWORD)0x037A000CL)

//
// MessageId: MSG_ERROR_PE32VIEW_CALLBACK_FAILED
//
// MessageText:
//
// Callback is failed
//
#define MSG_ERROR_PE32VIEW_CALLBACK_FAILED ((DWORD)0xC37A000DL)

//
// MessageId: MSG_ERROR_PE32VIEW_BREAKED_BY_CALLBACK
//
// MessageText:
//
// Breaked by callback
//
#define MSG_ERROR_PE32VIEW_BREAKED_BY_CALLBACK ((DWORD)0x037A000EL)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_DELAY_IMPORT
//
// MessageText:
//
// No delay import symbols
//
#define MSG_ERROR_PE32VIEW_NO_DELAY_IMPORT ((DWORD)0x037A000FL)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_CERTIFICATES
//
// MessageText:
//
// No certificates
//
#define MSG_ERROR_PE32VIEW_NO_CERTIFICATES ((DWORD)0x037A0010L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_DOTNET_DESC
//
// MessageText:
//
// No .NET descriptor
//
#define MSG_ERROR_PE32VIEW_NO_DOTNET_DESC ((DWORD)0x037A0011L)

//
// MessageId: MSG_ERROR_PE32VIEW_DOTNET_DESC_UNKNOWN
//
// MessageText:
//
// .NET descriptor is unknown
//
#define MSG_ERROR_PE32VIEW_DOTNET_DESC_UNKNOWN ((DWORD)0xC37A0012L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_EXPORT
//
// MessageText:
//
// No export symbols
//
#define MSG_ERROR_PE32VIEW_NO_EXPORT     ((DWORD)0x037A0013L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_RESOURCES
//
// MessageText:
//
// No resources
//
#define MSG_ERROR_PE32VIEW_NO_RESOURCES  ((DWORD)0x037A0014L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_VERSION
//
// MessageText:
//
// No version information
//
#define MSG_ERROR_PE32VIEW_NO_VERSION    ((DWORD)0x037A0015L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_VERSION_INFO
//
// MessageText:
//
// Version information is invalid
//
#define MSG_ERROR_PE32VIEW_INVALID_VERSION_INFO ((DWORD)0xC37A0016L)

//
// MessageId: MSG_ERROR_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO
//
// MessageText:
//
// FIXEDFILEINFO section of version information is invalid
//
#define MSG_ERROR_PE32VIEW_INVALID_VERSION_FIXEDFILEINFO ((DWORD)0xC37A0017L)

//
// MessageId: MSG_ERROR_PE32VIEW_NO_TYPELIB
//
// MessageText:
//
// No type library
//
#define MSG_ERROR_PE32VIEW_NO_TYPELIB    ((DWORD)0x037A0018L)

