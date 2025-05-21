#pragma once

#include <cassert>
#include <stdint.h>
#include <iostream>

using INT = int32_t;
using WORD = uint16_t;
using UINT = uint32_t;
using BYTE = uint8_t;
#ifdef _WIN32
using DWORD = unsigned long;
using LONG = long;
#else
using DWORD = uint32_t;
using LONG = int32_t;
#endif

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define ASSERT(condition) assert(condition)

#define Int32x32To64(a, b)  (((int64_t)((long)(a))) * ((int64_t)((long)(b))))
#define Int64ShraMod32(a, b) (((int64_t)(a)) >> (b))

inline void TRACE(const char* msg) 
{
    std::clog << msg << std::endl;
}
