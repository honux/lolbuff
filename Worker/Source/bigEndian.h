#ifndef __BIG_ENDIAN_H__
#define __BIG_ENDIAN_H__

const int bigEndianIdentifier = 1;
#define is_bigendian() ( (*(char*)&bigEndianIdentifier) == 0 )

#include "types.h"

namespace utils
{
    inline uint8 BigEndianU8 (uint8 value_)
    {
        return value_;
    }

    inline uint16 BigEndianU16 (uint16 value_)
    {
        if (is_bigendian())
        {
            return value_;
        }
        return (uint16)(((value_&0xFF)<<8)|((value_&0xFF00)>>8));
    }

    inline uint32 BigEndianU32 (uint32 value_)
    {
        if (is_bigendian())
        {
            return value_;
        }
        return (((value_&0xFF)<<24)|((value_&0xFF00)<<8)|((value_&0xFF0000)>>8)|((value_&0xFF000000)>>24));
    }

    inline uint64 BigEndianU64 (uint64 value_)
    {
        if (is_bigendian())
        {
            return value_;
        }
        return ((value_>>56) | 
        ((value_<<40) & 0x00FF000000000000) |
        ((value_<<24) & 0x0000FF0000000000) |
        ((value_<<8)  & 0x000000FF00000000) |
        ((value_>>8)  & 0x00000000FF000000) |
        ((value_>>24) & 0x0000000000FF0000) |
        ((value_>>40) & 0x000000000000FF00) |
        (value_<<56));
    }

    inline float BigEndianFloat (float value_)
    {
        if (is_bigendian())
        {
            return value_;
        }
        uint32 fool = BigEndianU32(*(uint32*)((void*)&value_));
        return *((float*)((void*)&fool));
    }

    inline double BigEndianDouble (double value_)
    {
        if (is_bigendian())
        {
            return value_;
        }
        uint64 fool = BigEndianU64(*(uint64*)((void*)&value_));
        return *((double*)((void*)&fool));
    }
}

#endif