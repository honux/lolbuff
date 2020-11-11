/********************************************************************//**
  @class ds::List
  Encapsulates a double-linked list.
  
  @class ds::FixedListMT
  Encapsulates a thread safe list with fixed maximum size.
  PS: It does not ensure data position remains the same.
*************************************************************************/

#pragma once

#include "types.h"
#include "compare.h"
#include "allocator.h"

#include <cassert>
#include <pthread.h>

namespace ds
{
    /************************************************************************
      List Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the element on List.
    /// @tparam Allocator Class that provides memory alocation to nodes on the list.
    /// Default is utils::MemoryPool.
    /// @tparam EqualityComparator Class that compares two elements through operator().
    /// Default is utils::Equal.
    ///
    template<typename Type, template<typename> class Allocator = utils::MemoryPool,
        typename EqualityComparator = utils::Equal<Type>>
    class List
    {
        /************************************************************************
          Node Struct Declaration
        *************************************************************************/
        struct Node
        {
            Node (Type const& _value)
            : value(_value)
            {
            }

            Type value;
            Node* prev;
            Node* next;
        };

    public:

        /************************************************************************
          ConstIterator Class Declaration
        *************************************************************************/
        class ConstIterator
        {
            friend class List;

        public:

            /// 
            /// Does nothing (optimized for performance).
            /// 
            ConstIterator ()
            {
            }

            ~ConstIterator ()
            {
            }

            bool operator== (ConstIterator it) const
            {
                return (it.m_node == m_node);
            }

            bool operator!= (ConstIterator it) const
            {
                return (it.m_node != m_node);
            }

            Type const& operator* () const
            {
                return m_node->value;
            }

            Type const& operator-> () const
            {
                return m_node->value;
            }

        protected:

            /// 
            /// Initializes the const iterator.
            /// @param[in] n Node of the const iterator.
            /// 
            ConstIterator(Node* n)
            : m_node(n)
            {
            }

        protected:

            Node* m_node;
        };

        /************************************************************************
          Iterator Class Declaration
        *************************************************************************/
        class Iterator
        {
            friend class List;

        public:

            /// 
            /// Does nothing (optimized for performance).
            /// 
            Iterator ()
            {
            }

            ~Iterator ()
            {
            }

            bool operator== (Iterator it) const
            {
                return (it.m_node == m_node);
            }

            bool operator!= (Iterator it) const
            {
                return (it.m_node != m_node);
            }

            operator ConstIterator& ()
            {
                return *(ConstIterator*)this;
            }

            Type& operator* ()
            {
                return m_node->value;
            }

            Type const& operator* () const
            {
                return m_node->value;
            }

            Type* operator-> ()
            {
                return &m_node->value;
            }

            Type const* operator-> () const
            {
                return &m_node->value;
            }

        protected:

            /// 
            /// Initializes the iterator.
            /// @param[in] n Node of the iterator.
            /// 
            Iterator(Node* n)
            : m_node(n)
            {
            }

        protected:

            Node* m_node;
        };

    public:
        
        /// 
        /// Initializes the list, preallocating the specified amount of slots.
        /// @param[in] sizeHint Amount of slots to preallocate.
        /// 
        explicit List (size_t sizeHint = 100)
        : m_allocator(sizeHint),
          m_start(NULL),
          m_end(NULL),
          m_size(0)
        {
        }

        /// 
        /// Destructor.
        /// 
        ~List ()
        {
        }

        /// 
        /// Clears the list.
        /// 
        void Clear ()
        {
            for (Iterator it = GetStart(); !IsEnd(it);)
            {
                Node* node = it.m_node;
                Next(&it);
                m_allocator.Release(node);
            }

            m_start = NULL;
            m_end = NULL;
            m_size = 0;
        }

        /// 
        /// Checks if the list contains a value.
        /// @param[in] value Value to be checked on the list.
        /// @return true if the specified value is contained on the list, false
        /// otherwise.
        /// 
        bool Contains (Type const& value) const
        {
            EqualityComparator compare;
            for (ConstIterator it = GetStart(); !IsEnd(it); Next(&it))
            {
                if (compare((*it), value))
                {
                    return true;
                }
            }

            return false;
        }

        /// 
        /// Finds the first occurrence of a value on the list and stores its position in
        /// an iterator.
        /// @param[in] value Value to be searched.
        /// @param[out] it Pointer to an iterator, used to store the position of the
        /// value found.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Type const& value, Iterator* it)
        {
            EqualityComparator compare;
            for (Iterator tmp = GetStart(); !IsEnd(tmp); Next(&tmp))
            {
                if (compare((*tmp), value))
                {
                    *it = tmp;
                    return true;
                }
            }

            return false;
        }

        /// 
        /// Finds the first occurrence of a value on the list that matches a selector and stores its
        /// position into an iterator.
        /// @tparam Selector Class with an operator() that receives a value and returns true shall it
        /// be selected, false otherwise.
        /// @param[in] selector Instance of the selector class.
        /// @param[out] it Pointer to an iterator used to store the position of the value.
        /// @return true if the value was found, false otherwise.
        /// 
        template<typename Selector>
        bool Find (Selector& selector, Iterator* it)
        {
            for (Iterator tmp = GetStart(); !IsEnd(tmp); Next(&tmp))
            {
                if (selector(*tmp))
                {
                    *it = tmp;
                    return true;
                }
            }

            return false;
        }

        /// 
        /// Gets the node allocator used by the list.
        /// @return A reference to the allocator.
        /// 
        Allocator<Node> const& GetAllocator () const
        {
            return m_allocator;
        }

        /// 
        /// Gets a iterator to the end of the list.
        /// @return An iterator to the end of the list.
        /// @remarks There is no valid value at this position.
        /// 
        Iterator GetEnd ()
        {
            return Iterator(NULL);
        }

        /// 
        /// Gets a const iterator to the end of the list.
        /// @return A const iterator to the end of the list.
        /// @remarks There is no valid value at this position.
        /// 
        ConstIterator GetEnd () const
        {
            return ConstIterator(NULL);
        }

        /// 
        /// Gets a reference to the first element of the list.
        /// @return A reference to the first element of the list.
        /// 
        Type& GetFirst ()
        {
            return m_start->value;
        }

        /// 
        /// Gets a const reference to the first element of the list.
        /// @return A const reference to the first element of the list.
        /// 
        Type const& GetFirst () const
        {
            return m_start->value;
        }

        /// 
        /// Gets a reference to the last element of the list.
        /// @return A reference to the last element of the list.
        /// 
        Type& GetLast ()
        {
            return m_end->value;
        }

        /// 
        /// Gets a const reference to the last element of the list.
        /// @return A const reference to the last element of the list.
        /// 
        Type const& GetLast () const
        {
            return m_end->value;
        }

        /// 
        /// Gets the size of the list.
        /// @return The size of the list.
        /// 
        size_t GetSize () const
        {
            return m_size;
        }

        /// 
        /// Gets an iterator positioned at the start of the list.
        /// @return An iterator positioned at the start of the list.
        /// 
        Iterator GetStart ()
        {
            return Iterator(m_start);
        }

        /// 
        /// Gets a const iterator positioned at the start of the list.
        /// @return A const iterator positioned at the start of the list.
        /// 
        ConstIterator GetStart () const
        {
            return ConstIterator(m_start);
        }

        /// 
        /// Inserts a value on the list, before the position specified by an iterator.
        /// @param[in] value Value to be inserted.
        /// @param[in] it Iterator that specifies the position where the value should
        /// be inserted. It will become the subsequent value on the list.
        /// @return true if the list has successfully allocated memory for the new value,
        /// false otherwise.
        /// 
        bool InsertAt (Type const& value, Iterator it)
        {
            Node* node = new(m_allocator) Node(value);

            // Memory allocation failed
            if (node == NULL)
            {
                return false;
            }

            node->next = it.m_node;
            _Insert(node, it);
            m_size++;
            return true;
        }

        /// 
        /// Inserts a value at the first position of the list.
        /// @param[in] value Value to be inserted.
        /// @return true if the list has successfully allocated memory for the new value,
        /// false otherwise.
        /// 
        bool InsertFirst (Type const& value)
        {
            return InsertAt(value, GetStart());
        }

        /// 
        /// Inserts a value at the end of the list.
        /// @param[in] value Value to be inserted.
        /// @return true if the list has successfully allocated memory for the new value,
        /// false otherwise.
        /// 
        bool InsertLast (Type const& value)
        {
            return InsertAt(value, GetEnd());
        }

        /// 
        /// Checks if the list is empty.
        /// @return true if the list is empty, false otherwise.
        /// 
        bool IsEmpty () const
        {
            return (m_size == 0);
        }

        /// 
        /// Checks if an iterator is at the end of the list.
        /// @param[in] it Iterator to check.
        /// @return true if the iterator is at the end of the list, false otherwise.
        /// 
        bool IsEnd (Iterator it) const
        {
            return (it.m_node == NULL);
        }

        /// 
        /// Checks if a const iterator is at the end of the list.
        /// @param[in] it `const` iterator to check.
        /// @return true if the iterator is at the end of the list, false otherwise.
        /// 
        bool IsEnd (ConstIterator it) const
        {
            return (it.m_node == NULL);
        }

        /// 
        /// Moves a value on the list.
        /// @param[in] from Iterator to the value that is to be moved. The value pointed to by the
        /// iterator must be valid. To move the last value, apply Previous() on a GetEnd() iterator.
        /// @param[in] to Iterator specifying where to move the value. The value will be inserted before
        /// the position specified by the iterator.
        /// @remarks This function should be used whenever possible to avoid extra memory allocations when a
        /// value is being removed only to be reinserted in the list.
        ///
        void Move (Iterator from, Iterator to)
        {
            assert(!IsEnd(from));
            
            if (from == to)
            {
                return;
            }

            Node* node = from.m_node;
            _Remove(&from);
            _Insert(node, to);
        }

        /// 
        /// Advances an iterator on the list.
        /// @param[in,out] it Pointer to the iterator to advance.
        /// 
        void Next (Iterator* it)
        {
            assert(it != NULL);
            assert(!IsEnd(*it));

            it->m_node = it->m_node->next;
        }

        /// 
        /// Advances a const iterator on the list.
        /// @param[in,out] it Pointer to the const iterator to advance.
        /// 
        void Next (ConstIterator* it) const
        {
            assert(it != NULL);
            assert(!IsEnd(*it));

            it->m_node = it->m_node->next;
        }

        /// 
        /// Rewinds an iterator on the list.
        /// @param[in,out] it Pointer to the iterator to rewind.
        /// 
        void Previous (Iterator* it)
        {
            assert(it != NULL);
            assert(*it != GetStart());

            if (!IsEnd(*it))
            {
                it->m_node = it->m_node->prev;
            }
            else
            {
                it->m_node = m_end;
            }
        }

        /// 
        /// Rewinds a const iterator on the list.
        /// @param[in,out] it Pointer to the const iterator to rewind.
        /// 
        void Previous (ConstIterator* it) const
        {
            assert(it != NULL);
            assert(*it != GetStart());

            if (!IsEnd(*it))
            {
                it->m_node = it->m_node->prev;
            }
            else
            {
                it->m_node = m_end;
            }
        }

        /// 
        /// Removes a value from the list.
        /// @param[in] value Value to be removed.
        /// 
        void Remove (Type const& value)
        {
            Iterator it;
            
            if (Find(value, &it))
            {
                RemoveAt(&it);
            }
        }

        /// 
        /// Removes a value whose position is specified by an iterator.
        /// @param[in,out] it Pointer to an iterator that specifies the position of the
        /// value on the list. The iterator is updated to point to the next element.
        /// @remarks After removing the value, the iterator is updated, such that any
        /// iterating operation can continue.
        /// 
        void RemoveAt (Iterator* it)
        {
            assert(it != NULL);

            Node* at = it->m_node;
            _Remove(it);
            m_allocator.Release(at);
            m_size--;
        }

        /// 
        /// Removes the first element from the list.
        /// 
        void RemoveFirst ()
        {
            Iterator it = GetStart();
            return RemoveAt(&it);
        }

        /// 
        /// Removes the last element from the list.
        /// 
        void RemoveLast ()
        {
            Iterator it = Iterator(m_end);
            return RemoveAt(&it);
        }

    protected:

        /// 
        /// Inserts a node in the list, before the position specified by an iterator.
        /// @param[in] node Node to be insert in the specified position.
        /// @param[in] where Iterator specifying where to add the node.
        /// @remarks This function does not acquire any memory or increase the size of the list, it simply
        /// inserts the node.
        ///
        void _Insert (Node* node, Iterator where)
        {
            if (!IsEnd(where))
            {
                Node* at = where.m_node;

                if (at->prev != NULL)
                {
                    at->prev->next = node;
                }
                else
                {
                    m_start = node;
                }

                node->prev = at->prev;
                node->next = at;
                at->prev = node;
            }
            else
            {
                if (m_end != NULL)
                {
                    m_end->next = node;
                }
                else if (m_start == NULL)
                {
                    m_start = node;
                }

                node->prev = m_end;
                node->next = NULL;
                m_end = node;
            }
        }

        /// 
        /// Removes a value from the list, based on an iterator.
        /// @param[in,out] it Pointer to an iterator that specifies the position of the
        /// value on the list.
        /// @remarks This function does not release any memory or decrease the size of the list, it simply
        /// removes the value.
        ///
        void _Remove (Iterator* it)
        {
            Node* at = it->m_node;

            if (at->prev != NULL)
            {
                at->prev->next = at->next;
            }
            else
            {
                m_start = m_start->next;
            }

            if (at->next != NULL)
            {
                at->next->prev = at->prev;
            }
            else
            {
                m_end = m_end->prev;
            }

            it->m_node = at->next;
        }

    protected:

        Allocator<Node> m_allocator;
        Node* m_start;
        Node* m_end;
        size_t m_size;

    private:

        List (const List& list);
        List& operator= (const List& list);
    };
    
    template <typename Type, size_t Size, typename EqualityComparator = utils::Equal<Type>>
    class FixedList
    {
    public:
        FixedList ()
        : m_size(0)
        {
        }
        
        ~FixedList ()
        {
        }
        
        bool Insert (Type _value)
        {
            if (m_size == Size)
            {
                return false;
            }
            
            m_data[m_size++] = _value;
            return true;
        }
        
        bool InsertUnique (Type _value)
        {
            if (m_size == Size || Contains(_value))
            {
                return false;
            }
            
            m_data[m_size++] = _value;
            return true;
        }

        bool Remove (Type _value)
        {
            size_t index;
            EqualityComparator compare;
                            
            for (index = 0; index < m_size; index++)
            {
                if (compare(m_data[index], _value))
                {
                    break;
                }
            }
            
            if (index == m_size)
            {
                return false;
            }
            
            m_data[index] = m_data[--m_size];
            
            return true;
        }
        
        Type PeekAndRemoveFirst ()
        {
            Type ret = NULL;
            if (m_size != 0)
            {
                ret = m_data[--m_size];
                return ret;
            }
            return ret;
        }

        bool Contains (Type _value)
        {
            EqualityComparator compare;
                            
            for (size_t index = 0; index < m_size; index++)
            {
                if (compare(m_data[index], _value))
                {
                    return true;
                }
            }
            return false;
        }

        size_t GetSize ()
        {
            return m_size;
        }

    protected:
        
    private:
        Type m_data[Size];
        size_t m_size;
    };

    template <typename Type, size_t Size, typename EqualityComparator = utils::Equal<Type>>
    class FixedListMT
    {
    public:
        FixedListMT ()
        : m_size(0),
          m_mutex(PTHREAD_MUTEX_INITIALIZER)
        {
        }
        
        ~FixedListMT ()
        {
        }
        
        bool Insert (Type _value)
        {
            pthread_mutex_lock(&m_mutex);
            
            if (m_size == Size)
            {
                pthread_mutex_unlock(&m_mutex);
                return false;
            }
            
            m_data[m_size++] = _value;
            pthread_mutex_unlock(&m_mutex);
            return true;
        }
        
        bool InsertUnique (Type _value)
        {
            pthread_mutex_lock(&m_mutex);
            
            if (m_size == Size || _Contains(_value))
            {
                pthread_mutex_unlock(&m_mutex);
                return false;
            }
            
            m_data[m_size++] = _value;
            pthread_mutex_unlock(&m_mutex);
            return true;
        }

        bool Remove (Type _value)
        {
            size_t index;
            EqualityComparator compare;
            pthread_mutex_lock(&m_mutex);
                            
            for (index = 0; index < m_size; index++)
            {
                if (compare(m_data[index], _value))
                {
                    break;
                }
            }
            
            if (index == m_size)
            {
                pthread_mutex_unlock(&m_mutex);
                return false;
            }
            
            m_data[index] = m_data[--m_size];
            
            pthread_mutex_unlock(&m_mutex);
            return true;
        }
        
        Type PeekAndRemoveFirst ()
        {
            Type ret = NULL;
            pthread_mutex_lock(&m_mutex);
            if (m_size != 0)
            {
                ret = m_data[--m_size];
                pthread_mutex_unlock(&m_mutex);
                return ret;
            }
            pthread_mutex_unlock(&m_mutex);
            return ret;
        }

        bool Contains (Type _value)
        {
            EqualityComparator compare;
            pthread_mutex_lock(&m_mutex);
                            
            for (size_t index = 0; index < m_size; index++)
            {
                if (compare(m_data[index], _value))
                {
                    pthread_mutex_unlock(&m_mutex);
                    return true;
                }
            }
            pthread_mutex_unlock(&m_mutex);
            return false;
        }

        size_t GetSize ()
        {
            return m_size;
        }

    protected:
        bool _Contains (Type _value)
        {
            EqualityComparator compare;
                            
            for (size_t index = 0; index < m_size; index++)
            {
                if (compare(m_data[index], _value))
                {
                    return true;
                }
            }
            return false;
        }
        
    private:
        Type m_data[Size];
        size_t m_size;
        pthread_mutex_t m_mutex;
    };
};
