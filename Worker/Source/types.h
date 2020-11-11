#pragma once

#include <stddef.h>

typedef signed char int8;
typedef short       int16;
typedef int         int32;
typedef long long   int64;

typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;
typedef unsigned long long  ulonglong;

typedef uchar       uint8;
typedef ushort      uint16;
typedef uint        uint32;
typedef ulonglong   uint64;

#ifdef _M_X64
    typedef double float_size_t;
#else
    typedef float float_size_t;
#endif

// _CACHE_ALIGN is used to optimize cache access in multiprocessor scenarios.
#define _CACHE_LINE  64
#ifndef _NO_CACHE_ALIGN
#   define _CACHE_ALIGN __declspec(align(_CACHE_LINE))
#else
#   define _CACHE_ALIGN
#endif

#define _THREAD_LOCAL __declspec(thread)

// _NO_DEFAULT_CASE is used to optimize a switch statement guaranteed to have no default case.
#ifdef _DEBUG
#   define _NO_DEFAULT_CASE   assert(0)
#else
#   define _NO_DEFAULT_CASE   __assume(0)
#endif