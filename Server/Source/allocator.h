#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include "types.h"

#include <new>
#include <exception>

#include <cassert>
#include <cstdlib>
#include <cmath>

namespace utils
{

    /************************************************************************
      Template Class Declaration
    *************************************************************************/
    #define _MEMORYBLOCK_INVALID_INDEX  0xFFFFFFFF
    ///
    /// @tparam Type Type of the object being allocated.
    ///
    template<typename Type>
    class MemoryBlock
    {
    public:

        /// 
        /// Initializes the memory block.
        /// @param[in] capacity Number of elements to be supported by the block. Must be greater 
        /// than zero.
        /// @throws std::bad_alloc Memory could not be allocated.
        /// 
        explicit MemoryBlock (size_t capacity)
        : m_acquiredCount(0),
          m_chunkCount(0),
          m_free(NULL)
        {
            assert(capacity > 0);

            m_buffer = new uint8[capacity * s_elemSize];
            m_chunkCount = capacity;
            _Reset(); // Initialize the buffers
        }

        /// 
        /// Initializes the memory block by moving another one.
        ///
        MemoryBlock (MemoryBlock&& block)
        : m_acquiredCount(block.m_acquiredCount),
          m_chunkCount(block.m_chunkCount),
          m_free(block.m_free),
          m_buffer(block.m_buffer)
        {
            block.m_buffer = NULL;
        }

        MemoryBlock& operator= (MemoryBlock&& block)
        {
            if (&block != this)
            {
                delete[] m_buffer;
                m_acquiredCount = block.m_acquiredCount;
                m_chunkCount = block.m_chunkCount;
                m_free = block.m_free;
                m_buffer = block.m_buffer;
                block.m_buffer = NULL;
            }
            return *this;
        }

        ~MemoryBlock ()
        {
            delete[] m_buffer;
        }

        /// 
        /// Acquires memory to be used by a new object.
        /// @return A pointer to the acquired memory area, or `NULL` if the block is full.
        /// @remarks The constructor will not be called. Use 'placement new' instead to construct
        /// the object.
        /// 
        Type* Acquire ()
        {
            if (m_free != NULL)
            {
                Type* ret = (Type*)m_free;
                m_free = *(uint8**)m_free;
                m_acquiredCount++;
                return ret;
            }
            return NULL;
        }

        /// 
        /// Checks if the specified memory address has been previously allocated by the memory block.
        /// @param[in] object Pointer with the address to be checked.
        /// @return true if the specified address has been previously allocated by the memory block,
        /// false otherwise.
        /// 
        bool Contains (Type const* object) const
        {
            return (object && (uint8*)object >= m_buffer 
                && (uint8*)object <= &m_buffer[s_elemSize*(m_chunkCount-1)]);
        }

        /// 
        /// Gets the number of acquired elements in the block.
        /// @return The count of acquired elements in the block.
        /// 
        size_t GetAcquiredCount () const
        {
            return m_acquiredCount;
        }

        /// 
        /// Gets the base of an address pointing to somewhere within an object.
        /// @param[in] ptr Address pointing to somewhere within an object.
        /// @return The base address of the object, or `NULL` if the specified address was not allocated
        /// by this block.
        /// @remarks This is useful, for example, to get the original acquired pointer of an array by passing 
        /// a pointer to any of its elements.
        /// 
        Type* GetBaseAddress (Type const* ptr) const
        {
            if (!Contains(ptr))
            {
                return NULL;
            }

            Type const* base = (Type const*)m_buffer;
            return (Type*)(base + (size_t)(ptr - base));
        }

        /// 
        /// Gets the total amount of elements that are supported by the block.
        /// @return The amount of elements supported by the block.
        /// 
        size_t GetCapacity () const
        {
            return m_chunkCount;
        }

        /// 
        /// Gets the number of free elements in the block.
        /// @return The number of free elements in the block.
        /// 
        size_t GetFreeCount () const
        {
            return (m_chunkCount - m_acquiredCount);
        }

        /// 
        /// Gets the total amount of memory used by the block in bytes.
        /// @return Memory used by the block in bytes.
        /// 
        size_t GetMemoryUsage () const
        {
            return s_elemSize * m_chunkCount;
        }

        /// 
        /// Checks if the memory block is full, that is, it cannot allocate more data.
        /// @return true if the memory block is full, false otherwise.
        /// 
        bool IsFull () const
        {
            return (m_free == NULL);
        }

        /// 
        /// Releases a previously acquired object and calls its destructor.
        /// @param[in] object Pointer to the memory area being released.
        /// 
        void Release (Type* object)
        {
            if (object == NULL)
            {
                return;
            }

            assert(Contains(object));

            object->~Type();
            
            *(uint8**)object = m_free;
            m_free = (uint8*)object;

            m_acquiredCount--;

            if (m_acquiredCount == 0) // improves locality by linearizing free nodes
            {
                _Reset();
            }
        }


        void ReleaseAll ()
        {
            m_acquiredCount = 0;
            _Reset();
        }

    protected:

        ///
        /// Resets the free nodes making them linear.
        ///
        void _Reset ()
        {
            uint8* last = &m_buffer[(m_chunkCount-1)*s_elemSize];
            for (uint8* addr = m_buffer; addr != last; addr += s_elemSize)
            {
                *(uint8**)addr = addr + s_elemSize;
            }
            *(uint8**)last = NULL;

            m_free = m_buffer;
        }

    protected:

        static const size_t s_elemSize = (sizeof(Type) >= sizeof(uint8*)) ? sizeof(Type) : sizeof(uint8*); ///< Size of the chunk.
        
        uint8* m_buffer; ///< Preallocated memory buffer.
        uint8* m_free; ///< Pointer to the next free node.
        size_t m_chunkCount; ///< Total capacity of the buffer.
        size_t m_acquiredCount; ///< Used capacity of the buffer.

    private:

        MemoryBlock (MemoryBlock const& block);
        MemoryBlock& operator= (MemoryBlock const& block);
    };

    /************************************************************************
      MemoryPool Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Type Type of the object being allocated.
    ///
    template<typename Type>
    class MemoryPool
    {
        struct BlockNode
        {
            /// 
            /// Initializes the block node of the memory pool.
            /// @param[in] chunkCount Chunk count of the memory block.
            /// 
            BlockNode (size_t chunkCount)
            : block(chunkCount),
              next(NULL)
            {
            }

            MemoryBlock<Type> block;
            BlockNode* next;
        };

    public:
        /// 
        /// Initializes the memory pool.
        /// @param[in] initialSize Number of elements to allocate initially. Must be greater
        /// than zero.
        /// @param[in] growFactor (Optional) When the memory pool needs to grow, it'll allocate a new memory
        /// block with the size of the last block created scaled by this growFactor. Must be
        /// equal or greater than 1. Default is 1.25.
        /// @throws std::bad_alloc Memory could not be allocated.
        /// 
        explicit MemoryPool (size_t initialSize, float growFactor = 1.25f)
        : m_chunkCount(initialSize),
          m_acquired(0),
          m_blockCount(1),
          m_growCount(0),
          m_growFactor(growFactor)
        {
            assert(initialSize > 0);
            assert(growFactor >= 1.0f);

            m_root = new BlockNode(initialSize);
        }

        /// 
        /// Initializes the memory pool by moving another one.
        ///
        MemoryPool (MemoryPool&& pool)
        : m_acquired(pool.m_acquired),
          m_blockCount(pool.m_blockCount),
          m_chunkCount(pool.m_chunkCount),
          m_growCount(pool.m_growCount),
          m_growFactor(pool.m_growFactor),
          m_root(pool.m_root)
        {
            pool.m_root = NULL;
        }

        MemoryPool& operator= (MemoryPool&& pool)
        {
            if (&pool != this)
            {
                BlockNode* node = m_root;
                while (node != NULL)
                {
                    BlockNode* next = node->next;
                    delete node;
                    node = next;
                }

                m_acquired = pool.m_acquired;
                m_blockCount = pool.m_blockCount;
                m_chunkCount = pool.m_chunkCount;
                m_growCount = pool.m_growCount;
                m_growFactor = pool.m_growFactor;
                m_root = pool.m_root;

                pool.m_root = NULL;
            }
            return *this;
        }

        /// 
        /// Releases all elements in the pool and destroys it.
        /// 
        ~MemoryPool ()
        {
            BlockNode* node = m_root;
            while (node != NULL)
            {
                BlockNode* next = node->next;
                delete node;
                node = next;
            }
        }

        /// 
        /// Acquires memory to be used by a new object.
        /// @return A pointer to the acquired memory area, or `NULL` if there is not enough memory.
        /// 
        Type* Acquire ()
        {
            for (BlockNode* node = m_root; node != NULL; node = node->next)
            {
                if (!node->block.IsFull())
                {
                    m_acquired++;
                    return node->block.Acquire();
                }
            }

            if (!_CreateBlock()) // Bad allocation
            {
                return NULL;
            }

            m_acquired++;
            return m_root->block.Acquire();
        }

        /// 
        /// Gets the number of acquired elements in the pool.
        /// @return The count of acquired elements in the pool.
        /// 
        size_t GetAcquiredCount () const
        {
            return m_acquired;
        }

        /// 
        /// Gets the number of memory blocks that the pool contains.
        /// @return The number of memory blocks that the pool contains.
        /// 
        size_t GetBlockCount () const
        {
            return m_blockCount;
        }

        /// 
        /// Gets the number of elements that the memory pool currently supports.
        /// @return The number of elements that the memory pool currently supports.
        /// 
        size_t GetCapacity () const
        {
            return m_chunkCount;
        }

        /// 
        /// Gets the number of free elements in the pool.
        /// @return The number of free elements in the pool.
        /// 
        size_t GetFreeCount () const
        {
            return (m_chunkCount - m_acquired);
        }

        /// 
        /// Gets the number of times the pool has grown.
        /// @return The number of times the pool has grown.
        /// 
        size_t GetGrowCount () const
        {
            return m_growCount;
        }

        /// 
        /// Gets the grow factor of the pool.
        /// @return The grow factor of the pool.
        /// 
        float GetGrowFactor () const
        {
            return m_growFactor;
        }

        /// 
        /// Gets the total amount of memory used by the block in bytes.
        /// @return Memory used by the block in bytes.
        /// 
        size_t GetMemoryUsage () const
        {
            size_t totalMemory = 0;
            for (BlockNode* node = m_root; node != NULL; node = node->next)
            {
                totalMemory += node->block.GetMemoryUsage();
            }
            return totalMemory;
        }

        /// 
        /// Releases a previously acquired object.
        /// @param[in] object Pointer to the memory area being released.
        /// 
        void Release (Type* object)
        {
            if (object == NULL)
            {
                return;
            }

            BlockNode* node;
            BlockNode* prev;

            for (prev = NULL, node = m_root; node != NULL; node = node->next)
            {
                if (node->block.Contains(object))
                {
                    m_acquired--;
                    node->block.Release(object);
                    break;
                }

                prev = node;
            }

            // If we have more than one block and the oldest one is smaller and empty, shrink the pool.
            if (prev != NULL && node != NULL && node->next == NULL && node->block.GetAcquiredCount() == 0
                && node->block.GetCapacity() < prev->block.GetCapacity())
            {
                m_chunkCount -= node->block.GetCapacity();
                delete node;
                prev->next = NULL;
                m_blockCount--;
            }
        }

        /// 
        /// Sets the grow factor of the pool.
        /// @param[in] growFactor Desired grow factor of the pool. Must be equal or greater than 1.
        /// 
        void SetGrowFactor (float growFactor)
        {
            assert(growFactor >= 1.0f);
            m_growFactor = growFactor;
        }

        void ReleaseAll ()
        {
            BlockNode* node;
            BlockNode* prev;

            for (prev = NULL, node = m_root; node != NULL; node = node->next)
            {
                node->block.ReleaseAll();
            }

            m_acquired = 0;
        }

    protected:

        /// 
        /// Creates a new memory block.
        /// @return true if the memory pool has successfully allocated the new block,
        /// false otherwise.
        /// 
        bool _CreateBlock () throw()
        {
            size_t chunkCount = (size_t)ceil((float_size_t)m_root->block.GetCapacity() * m_growFactor);
            assert(chunkCount > 0);

            BlockNode* node;
            try
            {
                node = new BlockNode(chunkCount);
            }
            catch (...)
            {
            	return false;
            }

            node->next = m_root;
            m_root = node;
            m_chunkCount += chunkCount;
            m_blockCount++;
            m_growCount++;
            return true;
        }

    protected:

        BlockNode* m_root;
        size_t m_chunkCount;
        size_t m_acquired;
        size_t m_blockCount;
        size_t m_growCount;
        float m_growFactor;

    private:

        MemoryPool (MemoryPool const& pool);
        MemoryPool& operator= (MemoryPool const& pool);
    };
}

/************************************************************************
  Inline Methods
*************************************************************************/

template<typename Type>
inline void* operator new (size_t, utils::MemoryBlock<Type>* block)
{
    return block->Acquire();
}

template<typename Type>
inline void operator delete (void* object, utils::MemoryBlock<Type>* block)
{
    return block->Release((Type*)object);
}

template<typename Type>
inline void* operator new (size_t, utils::MemoryBlock<Type>& block)
{
    return block.Acquire();
}

template<typename Type>
inline void operator delete (void* object, utils::MemoryBlock<Type>& block)
{
    return block.Release((Type*)object);
}

template<typename Type>
inline void* operator new (size_t, utils::MemoryPool<Type>* pool)
{
    return pool->Acquire();
}

template<typename Type>
inline void operator delete (void* object, utils::MemoryPool<Type>* pool)
{
    return pool->Release((Type*)object);
}

template<typename Type>
inline void* operator new (size_t, utils::MemoryPool<Type>& pool)
{
    return pool.Acquire();
}

template<typename Type>
inline void operator delete (void* object, utils::MemoryPool<Type>& pool)
{
    return pool.Release((Type*)object);
}

#endif