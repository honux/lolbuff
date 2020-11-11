/********************************************************************//**
  @class utils::Less
  Defines an overloaded operator() for less-than comparison.

  @class utils::Equal
  Defines an overloaded operator() for equality comparison.

  @class utils::Greater
  Defines an overloaded operator() for greater-than comparison.
*************************************************************************/

#pragma once

#include <cmath>
#include <cstring>

namespace utils
{
    /************************************************************************
      Less Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the element.
    ///
    template<typename Type>
    class Less
    {
    public:

        bool operator() (Type const& first, Type const& second) const
        {
            return (first < second);
        }
    };

    /************************************************************************
      Equal Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the element.
    ///
    template<typename Type>
    class Equal
    {
    public:

        bool operator() (Type const& first, Type const& second) const
        {
            return (first == second);
        }
    };

    /************************************************************************
      Equal Template Class Specialization
    *************************************************************************/
    template<>
    class Equal<char*>
    {
    public:

        bool operator() (char* first, char* second) const
        {
            return (strcmp(first, second) == 0);
        }
    };

    template<>
    class Equal<char const*>
    {
    public:

        bool operator() (char const* first, char const* second) const
        {
            return (strcmp(first, second) == 0);
        }
    };

    /************************************************************************
      Greater Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the element.
    ///
    template<typename Type>
    class Greater
    {
    public:

        bool operator() (Type const& first, Type const& second) const
        {
            return (first > second);
        }
    };

}
