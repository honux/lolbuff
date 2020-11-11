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
