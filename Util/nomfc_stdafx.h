#pragma once

#include <cassert>
#include <stdint.h>
#include <iostream>

#ifndef _WINDEF_
using INT = int32_t;
using WORD = uint16_t;
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

#define Int32x32To64(a, b)  (((int64_t)((long)(a))) * ((int64_t)((long)(b))))
#define Int64ShraMod32(a, b) (((int64_t)(a)) >> (b))


inline void TRACE(const char* msg) 
{
    std::clog << msg << std::endl;
}

#endif
