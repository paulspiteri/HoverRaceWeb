// // stdafx.h : include file for standard system include files,
// //  or project specific include files that are used frequently, but
// //      are changed infrequently
// //

// #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

// #include <afxwin.h>         // MFC core and standard components
// #include <afxext.h>         // MFC extensions
// #include <afxtempl.h>

// #include <typeinfo>

// #ifndef _AFX_NO_AFXCMN_SUPPORT
// #include <afxcmn.h>			// MFC support for Windows 95 Common Controls
// #endif // _AFX_NO_AFXCMN_SUPPORT


#include <cassert>
#include <stdint.h>

#ifndef _WINDEF_
using INT = int32_t;
using DWORD = uint32_t;
using LONG = int32_t;
using UINT = uint32_t;
using BYTE = uint8_t;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

using HWND = void*;

#define ASSERT(condition) assert(condition)

#define Int32x32To64(a, b)  (((__int64)((long)(a))) * ((__int64)((long)(b))))

#endif
