/************************************************************************
  Math Functions
  Provides general math methods.
*************************************************************************/

#pragma once

#include "types.h"

#include <cmath>

namespace utils
{

    /// 
    /// Returns the absolute value of a number.
    /// @param[in] a Signed number.
    /// @return Absolute value of the number.
    ///
    template<class Type>
    inline Type Abs (Type a)
    {
        return ((a >= 0) ? a : (-a));
    }

    /// 
    /// Rounds a number up to the nearest power of two.
    /// @param[in] number Number to be rounded up.
    /// @return The number rounded up to the nearest power of two.
    /// 
    inline uint32 CeilToPowerOfTwo (uint32 number)
    {
        number--;
        number |= (number >> 1);
        number |= (number >> 2);
        number |= (number >> 4);
        number |= (number >> 8);
        number |= (number >> 16);
        number++;
        return number;
    }

    /// 
    /// Rounds a number up to the nearest power of two.
    /// @param[in] number Number to be rounded up.
    /// @return The number rounded up to the nearest power of two.
    /// 
    inline uint64 CeilToPowerOfTwo64 (uint64 number)
    {
        number--;
        number |= (number >> 1);
        number |= (number >> 2);
        number |= (number >> 4);
        number |= (number >> 8);
        number |= (number >> 16);
        number |= (number >> 32);
        number++;
        return number;
    }

    /// 
    /// Returns a value clamped to the specified range.
    /// @param[in] value Value to be clamped.
    /// @param[in] min Minimum value of the range.
    /// @param[in] max Maximum value of the range.
    /// @return min if value < min, max if value > max, or value otherwise.
    /// 
    template<class Type>
    inline Type Clamp (Type value, Type min, Type max)
    {
        if (value < min)
        {
            return min;
        }
        else if (value > max)
        {
            return max;
        }
        return value;
    }

    /// 
    /// Returns the greater of two values.
    /// @param[in] a First value to be compared.
    /// @param[in] b Second value to be compared.
    /// @return a if a > b, b otherwise.
    /// 
    template<class Type>
    inline Type Max (Type a, Type b)
    {
        return ((a > b) ? a : b);
    }

    /// 
    /// Returns the lesser of two values.
    /// @param[in] a First value to be compared.
    /// @param[in] b Second value to be compared.
    /// @return a if a < b, b otherwise.
    /// 
    template<class Type>
    inline Type Min (Type a, Type b)
    {
        return ((a > b) ? b : a);
    }

}
