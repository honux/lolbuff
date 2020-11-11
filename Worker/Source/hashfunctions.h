/********************************************************************//**
  @class utils::HashRaw
  Functor that implements a hash function by returning the raw key value
  converted explicitly to an unsigned integer.

  @class utils::HashDefault
  Functor that implements the most suitable hash function for the given
  type. It uses a Jenkins' full avalanche hash for 32-bit integers, and 
  the MurmurHash3 for all other types.
*************************************************************************/

#ifndef _HASHFUNCTIONS_H_
#define _HASHFUNCTIONS_H_

#include "types.h"
#include "string.h"

#include <cstring>

namespace utils
{
    /************************************************************************
      MurmurHash3 Function Declarations
    *************************************************************************/
    /// 
    /// Gets a 32-bit hash value.
    /// @param[in] key Key to be hashed.
    /// @param[in] len Size of the key.
    /// @param[in] seed Seed value. Default is zero.
    /// @return 32-bit key hash value.
    /// 
    uint32 MurmurHash3_x86_32  (void const* key, int len, uint32 seed = 0);

    /// 
    /// Gets a 64-bit hash value.
    /// @param[in] key Key to be hashed.
    /// @param[in] len Size of the key.
    /// @param[in] seed Seed value. Default is zero.
    /// @return 64-bit key hash value.
    /// 
    uint64 MurmurHash3_x64_64 (void const* key, int len, uint32 seed = 0);

    /************************************************************************
      Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the key.
    ///
    template<typename Key>
    class HashRaw
    {
    public:

        uint operator() (const Key key) const
        {
            return (uint)key;
        }
    };

    /************************************************************************
      Template Class Declaration
    *************************************************************************/
    template<typename Key>
    ///
    /// @tparam Type Type of the key.
    ///
    class HashDefault
    {
    public:

        size_t operator() (const Key key) const
        {
#ifdef _M_X64
            return MurmurHash3_x64_64((char const*)&key, sizeof(Key));
#else
            return MurmurHash3_x86_32((char const*)&key, sizeof(Key));
#endif
        }
    };

    /************************************************************************
      Template Class Specialization
    *************************************************************************/
    template<>
    class HashDefault<uint>
    {
    public:

        uint operator() (uint key) const
        {
            key = (key+0x7ed55d16) + (key<<12);
            key = (key^0xc761c23c) ^ (key>>19);
            key = (key+0x165667b1) + (key<<5);
            key = (key+0xd3a2646c) ^ (key<<9);
            key = (key+0xfd7046c5) + (key<<3);
            key = (key^0xb55a4f09) ^ (key>>16);
            return key;
        }
    };

    /************************************************************************
      Template Class Specialization
    *************************************************************************/
    template<>
    class HashDefault<int>
    {
    public:

        uint operator() (int _key) const
        {
            uint key = (uint)_key;
            key = (key+0x7ed55d16) + (key<<12);
            key = (key^0xc761c23c) ^ (key>>19);
            key = (key+0x165667b1) + (key<<5);
            key = (key+0xd3a2646c) ^ (key<<9);
            key = (key+0xfd7046c5) + (key<<3);
            key = (key^0xb55a4f09) ^ (key>>16);
            return key;
        }
    };
    
    /************************************************************************
      Template Class Specialization
    *************************************************************************/
#ifndef _M_X64
    template<typename Key>
    class HashDefault<Key*>
    {
    public:

        uint operator() (Key* _key) const
        {
            uint key = (uint)_key;
            key = (key+0x7ed55d16) + (key<<12);
            key = (key^0xc761c23c) ^ (key>>19);
            key = (key+0x165667b1) + (key<<5);
            key = (key+0xd3a2646c) ^ (key<<9);
            key = (key+0xfd7046c5) + (key<<3);
            key = (key^0xb55a4f09) ^ (key>>16);
            return key;
        }
    };
#endif

    /************************************************************************
      Template Class Specialization
    *************************************************************************/
    template<>
    class HashDefault<char*>
    {
    public:

        size_t operator() (char* key) const
        {
#ifdef _M_X64
            return MurmurHash3_x64_64(key, (int)strlen(key));
#else
            return MurmurHash3_x86_32(key, (int)strlen(key));
#endif
        }
    };

    /************************************************************************
      Template Class Specialization
    *************************************************************************/
    template<>
    class HashDefault<char const*>
    {
    public:

        size_t operator() (char const* key) const
        {
#ifdef _M_X64
            return MurmurHash3_x64_64(key, (int)strlen(key));
#else
            return MurmurHash3_x86_32(key, (int)strlen(key));
#endif
        }
    };

    /************************************************************************
      Template Class Specialization
    *************************************************************************/
}

#endif
