/********************************************************************//**
  @class ds::FixedArray
  Provides an array with a constant size.

  @class ds::Array
  Provides an array with a size set in run-time.
*************************************************************************/

#pragma once

#include <cassert>

namespace ds
{
/************************************************************************
  Array Class Declaration
*************************************************************************/
    ///
    /// @tparam Type Type of the elements in the array.
    ///
    template<class Type>
    class Array
    {

    public:

        /// 
        /// Constructs an empty array.
        ///
        Array ()
        : m_size(0),
          m_data(NULL),
          m_ownPointer(false)
        {
        }

        /// 
        /// Constructs an array of the specified size.
        /// @param[in] size Size of the array.
        ///
        Array (size_t size)
        : m_size(size),
          m_ownPointer(true)
        {
            m_data = new Type[size];
        }

        /// 
        /// Copy-constructs the array from another.
        /// @param[in] array Array to be copied from.
        ///
        Array (Array const& array)
        : m_ownPointer(true)
        {
            m_data = new Type[array.m_size];
            m_size = array.m_size;
            for (size_t i = 0; i < m_size; i++)
            {
                m_data[i] = array[i];
            }
        }

        /// 
        /// Copy-assigns the array from another.
        /// @param[in] array Array to be copied from.
        ///
        Array& operator= (Array const& array)
        {
            if (this != &array)
            {
                Clear();
                m_data = new Type[array.m_size];
                m_size = array.m_size;
                for (size_t i = 0; i < m_size; i++)
                {
                    m_data[i] = array[i];
                }
            }
            return *this;
        }

        /// 
        /// Move-constructs the array from another.
        /// @param[in] array Array to be moved from.
        ///
        Array (Array&& array)
        : m_ownPointer(true)
        {
            m_data = array.m_data;
            m_size = array.m_size;
            array.m_data = NULL;
            array.m_size = 0;
        }

        /// 
        /// Move-assigns the array from another.
        /// @param[in] array Array to be moved from.
        ///
        Array& operator= (Array&& array)
        {
            if (this != &array)
            {
                delete[] m_data;
                m_data = array.m_data;
                m_size = array.m_size;
                array.m_data = NULL;
                array.m_size = 0;
            }
            return *this;
        }

        /// 
        /// Destructor.
        ///
        ~Array ()
        {
            if (m_ownPointer)
            {
                delete[] m_data;
            }
        }

        /// 
        /// Empties the array by deleting its data.
        ///
        void Clear ()
        {
            delete[] m_data;
            m_data = NULL;
            m_size = 0;
        }

        /// 
        /// Gets the array data buffer.
        /// @return Pointer to the data buffer.
        ///
        Type* GetData ()
        {
            return (Type*)m_data;
        }

        Type const* GetData() const
        {
            return (Type const*)m_data;
        }

        /// 
        /// Gets the element at the specified index.
        /// @param[in] index Index of the element. Must be less than the current size.
        /// @return Reference to the element at the specified index.
        ///
        Type& GetElement (size_t index)
        {
            assert(index < m_size);
            return m_data[index];
        }

        Type const& GetElement (size_t index) const
        {
            assert(index < m_size);
            return m_data[index];
        }

        /// 
        /// Gets the current size of the array.
        /// @return Size of the array.
        ///
        size_t GetSize () const
        {
            return m_size;
        }

        /// 
        /// Sets the value of the element at the specified index.
        /// @param[in] index Index of the element. Must be less than the current size.
        /// @param[in] value Value to be set.
        ///
        void SetElement (size_t index, Type value)
        {
            assert(index < m_size);
            m_data[index] = value;
        }

        /// 
        /// Sets the size of the array by deleting current data and allocating a new buffer.
        /// @param[in] size Size to be set.
        ///
        void SetSize (size_t size)
        {
            m_ownPointer = true;
            delete[] m_data;
            m_data = new Type[size];
            m_size = size;
        }

        void SetBuffer (Type* target_, size_t size_)
        {
            if (m_ownPointer)
            {
                delete[] m_data;
            }
            m_data = target_;
            m_size = size_;
        }

        operator Type* ()
        {
            return m_data;
        }

        operator Type const* () const
        {
            return (Type const*)m_data;
        }
        
    protected:

        Type* m_data;
        size_t m_size;
        bool m_ownPointer;
    };
}
