#pragma once

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

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE (HWND);

#define ASSERT(condition) assert(condition)

#define Int32x32To64(a, b)  (((__int64)((long)(a))) * ((__int64)((long)(b))))
#define Int64ShraMod32(a, b) (((__int64)(a)) >> (b))

#define TRACE              /* TODO */

#endif
