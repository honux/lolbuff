/********************************************************************//**
  @class ds::Map
  Provides a hash map that stores unique key-value pairs.
  INSERT: O(1)
  DELETE: O(1)
  SEARCH: O(1)
  @remarks The values in the map are unordered.
  If a custom KeyEqualityComparator is used, be sure to set the HashFunction
  accordingly, since by default it hashes over the binary representation
  of the key.
*************************************************************************/

#ifndef _MAP_H_
#define _MAP_H_

#include "compare.h"
#include "allocator.h"
#include "hashfunctions.h"

#include <cassert>

namespace ds
{
    /************************************************************************
      MapSizePolicyDefault Class Declaration
    *************************************************************************/
    class MapSizePolicyDefault
    {
    public:

        inline static size_t GetBucketIndex (size_t hash, size_t buckets)
        {
            return (hash % buckets);
        }
    };

    /************************************************************************
      MapSizePolicyPowerOfTwo Class Declaration
    *************************************************************************/
    class MapSizePolicyPowerOfTwo
    {
    public:

        inline static size_t GetBucketIndex (size_t hash, size_t buckets)
        {
            return (hash & (buckets - 1));
        }
    };

    /************************************************************************
      Map Template Class Declaration
    *************************************************************************/
    ///
    /// @tparam Key Type of the value's key.
    /// @tparam Value Type of the value on map which will be associated with the key.
    /// @tparam HashFunction Class that provides the hash of a key.
    /// Default is utils::HashDefault<Key>.
    /// @tparam Allocator Class that provides memory alocation to the elements on the map.
    /// Default is utils::MemoryPool.
    /// @tparam SizePolicy Defines the size policy of the map. If the size is guaranteed to be a
    /// power of two, use MapSizePolicyPowerOfTwo, else use MapSizePolicyDefault.
    /// Default is MultiMapSizePolicyDefault.
    /// @tparam KeyEqualityComparator Class that compares two keys through operator().
    /// Default is utils::Equal.
    /// @tparam ValueEqualityComparator Class that compares two values through operator().
    /// Default is utils::Equal.
    ///
    template<typename Key, typename Value,
        typename HashFunction = utils::HashDefault<Key>,
        template<typename> class Allocator = utils::MemoryPool,
        typename SizePolicy = MapSizePolicyDefault,
        typename KeyEqualityComparator = utils::Equal<Key>,
        typename ValueEqualityComparator = utils::Equal<Value>>
    class Map
    {

        /************************************************************************
          Record Struct Declaration
        *************************************************************************/
        struct Record
        {
            /// 
            /// Initializes the record.
            /// @param[in] k Key of the record.
            /// @param[in] v Value of the record.
            /// 
            Record (Key k, Value v)
            : key(k),
              value(v)
            {
            }

        public:

            const Key key;
            Value value;
        };

        /************************************************************************
          Node Struct Declaration
        *************************************************************************/
        struct Node
        {
            /// 
            /// Initializes the node.
            /// @param[in] k Key of the record of the node.
            /// @param[in] v Value of the record of the node.
            /// 
            Node (Key k, Value v)
            : record(k, v),
              next(NULL)
            {
            }

        public:

            Record record;
            Node* next;
        };

    public:

        /************************************************************************
          ConstIterator Class Declaration
        *************************************************************************/
        class ConstIterator
        {
            friend class Map;

        public:

            ConstIterator ()
            {
            }

            ~ConstIterator ()
            {
            }

            bool operator== (ConstIterator const& cit) const
            {
                return (m_node == cit.m_node);
            }

            bool operator!= (ConstIterator const& cit) const
            {
                return (m_node != cit.m_node);
            }

            const Record& operator* () const
            {
                return m_node->record;
            }

            const Record* operator-> () const
            {
                return &m_node->record;
            }

        protected:

            /// 
            /// Initializes the const iterator.
            /// @param[in] i Bucket index of the node.
            /// @param[in] n Node specifying the current position of the const
            /// iterator.
            /// @param[in] p Node specifying the previous position of the const
            /// iterator.
            /// 
            ConstIterator (size_t i, Node* n, Node* p)
            : m_index(i), 
              m_node(n), 
              m_prev(p)
            {
            }

        protected:

            size_t m_index;
            Node* m_node;
            Node* m_prev;
        };

        /************************************************************************
          Iterator Class Declaration
        *************************************************************************/
        class Iterator
        {
            friend class Map;

        public:

            Iterator ()
            {
            }

            ~Iterator ()
            {
            }

            bool operator== (const Iterator& it) const
            {
                return (m_node == it.m_node);
            }

            bool operator!= (const Iterator& it) const
            {
                return (m_node != it.m_node);
            }

            operator ConstIterator& ()
            {
                return *(ConstIterator*)this;
            }

            Record& operator* ()
            {
                return m_node->record;
            }

            const Record& operator* () const
            {
                return m_node->record;
            }

            Record* operator-> ()
            {
                return &m_node->record;
            }

            const Record* operator-> () const
            {
                return &m_node->record;
            }

        protected:

            /// 
            /// Initializes the iterator.
            /// @param[in] i Bucket index of the node.
            /// @param[in] n Node specifying the current position of the iterator.
            /// @param[in] p Node specifying the previous position of the iterator.
            /// 
            Iterator (size_t i, Node* n, Node* p)
            : m_index(i), 
              m_node(n), 
              m_prev(p)
            {
            }

        protected:

            size_t m_index;
            Node* m_node;
            Node* m_prev;
        };

    public:

        /// 
        /// Initializes the map.
        /// @param[in] bucketCount Amount of buckets to be created in the hash table, 
        /// and the amount of nodes to preallocate. Must be greater than zero.
        /// 
        explicit Map (size_t bucketCount)
        : m_bucketCount(bucketCount),
          m_size(0),
          m_allocator(bucketCount)
        {
            assert(bucketCount > 0);

            m_buckets = new Node*[bucketCount];
            memset(m_buckets, 0, (sizeof(Node*) * bucketCount));
        }

        /// 
        /// Initializes the map.
        /// @param[in] bucketCount Amount of buckets to be created in the hash table. Must be 
        /// greater than zero.
        /// @param[in] startNodeCount Amount of nodes as a size hint. Must be greater than zero.
        /// 
        explicit Map (size_t bucketCount, size_t startNodeCount)
        : m_bucketCount(bucketCount),
          m_size(0),
          m_allocator(startNodeCount)
        {
            assert(bucketCount > 0);
            assert(startNodeCount > 0);

            m_buckets = new Node*[bucketCount];
            memset(m_buckets, 0, (sizeof(Node*) * bucketCount));
        }

        ~Map ()
        {
            delete[] m_buckets;
        }

        /// 
        /// Clears the map by removing all key-value pairs.
        /// 
        void Clear ()
        {
            for (Iterator it = GetStart(); !IsEnd(it);)
            {
                Node* node = it.m_node;
                Next(&it);
                m_allocator.Release(node);
            }

            memset(m_buckets, 0, (sizeof(Node*) * m_bucketCount));
            m_size = 0;
        }

        /// 
        /// Checks if the map contains a value with the specified key.
        /// @param[in] key Key of the value to be checked.
        /// @return true if the specified key is in the map, false otherwise.
        /// 
        bool Contains (Key key) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    return true;
                }
            }

            return false;
        }

        /// 
        /// Checks if the given key is associated with the specified value in the map.
        /// @param[in] key Key to be checked.
        /// @param[in] value Value that must be associated with the key.
        /// @return true if the specified key-value pair is in the map, false otherwise.
        /// 
        bool Contains (Key key, Value value) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            ValueEqualityComparator valueCompare;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key) && valueCompare(node->record.value, value))
                {
                    return true;
                }
            }

            return false;
        }

        /// 
        /// Finds a value in the map and stores it in the specified reference.
        /// @param[in] key Key of the value being searched.
        /// @param[in] keyHash Previously calculated key hash.
        /// @param[out] value Pointer to a variable used to store the found value.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, uint keyHash, Value* value) const
        {
            size_t index = _GetBucketIndexByHash(keyHash);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    *value = node->record.value;
                    return true;
                }

                prev = node;
            }

            return false;
        }

        /// 
        /// Finds a value in the map and stores it in the specified reference.
        /// @param[in] key Key of the value being searched.
        /// @param[in] keyHash Previously calculated key hash.
        /// @param[out] value Pointer to a const value pointer used to store the address of the value.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, uint keyHash, Value const** value) const
        {
            size_t index = _GetBucketIndexByHash(keyHash);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    *value = &node->record.value;
                    return true;
                }

                prev = node;
            }

            return false;
        }

        /// 
        /// Finds a value in the map and stores its position in an iterator.
        /// @param[in] key Key of the value being searched.
        /// @param[in] keyHash Previously calculated key hash.
        /// @param[out] it Pointer to an iterator, used to store the position of the
        /// value found.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, uint keyHash, Iterator* it) const
        {
            size_t index = _GetBucketIndexByHash(keyHash);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    it->m_index = index;
                    it->m_prev = prev;
                    it->m_node = node;
                    return true;
                }

                prev = node;
            }

            it->m_node = NULL;
            return false;
        }

        /// 
        /// Finds a value in the map and returns a pointer to it's value
        /// @param[in] key Key of the value being searched.
        /// @param[out] value Pointer to a variable used to store the found value.
        /// @return value Pointer to it's value or NULL
        /// 
        Value* Find (Key key) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    return &node->record.value;
                }

                prev = node;
            }

            return NULL;
        }

        /// 
        /// Finds a value in the map and stores it in the specified reference.
        /// @param[in] key Key of the value being searched.
        /// @param[out] value Pointer to a variable used to store the found value.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, Value* value) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    *value = node->record.value;
                    return true;
                }

                prev = node;
            }

            return false;
        }

        /// 
        /// Finds a value in the map and stores it in the specified reference.
        /// @param[in] key Key of the value being searched.
        /// @param[out] value Pointer to a const value pointer used to store the address of the value.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, Value const** value) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    *value = &node->record.value;
                    return true;
                }

                prev = node;
            }

            return false;
        }

        /// 
        /// Finds a value in the map and stores its position in an iterator.
        /// @param[in] key Key of the value being searched.
        /// @param[out] it Pointer to an iterator, used to store the position of the
        /// value found.
        /// @return true if the value was found, false otherwise.
        /// 
        bool Find (Key key, Iterator* it) const
        {
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            Node* prev = NULL;
            for (Node* node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    it->m_index = index;
                    it->m_prev = prev;
                    it->m_node = node;
                    return true;
                }

                prev = node;
            }

            it->m_node = NULL;
            return false;
        }

        /// 
        /// Gets the node allocator used by the map.
        /// @return A reference to the allocator.
        /// 
        Allocator<Node> const& GetAllocator () const
        {
            return m_allocator;
        }

        /// 
        /// Gets the number of buckets in the hash table.
        /// @return The number of buckets in the hash table.
        /// 
        size_t GetBucketCount () const
        {
            return m_bucketCount;
        }

        /// 
        /// Gets an array with the number of values stored in each bucket (collisions).
        /// @param[out] buffer Array of size_t with GetBucketCount() elements that will
        /// contain the number of values stored in each corresponding bucket.
        /// 
        void GetCollisionCounts (size_t* buffer)
        {
            for (size_t i = 0; i < m_bucketCount; i++)
            {
                Node* node = m_buckets[i];
                if (node)
                {
                    size_t count = 0;
                    do 
                    {
                        count++;
                    } while (node = node->next);
                    buffer[i] = count;
                }
                else
                {
                    buffer[i] = 0;
                }
            }
        }

        /// 
        /// Gets an iterator positioned at the end of the map.
        /// @return Iterator positioned at the end of the map.
        /// @remarks There is no valid value at this position.
        ///
        Iterator GetEnd ()
        {
            return Iterator(0, NULL, NULL);
        }

        /// 
        /// Gets a const iterator positioned at the end of the map.
        /// @return Const iterator positioned at the end of the map.
        /// @remarks There is no valid value at this position.
        ///
        ConstIterator GetEnd () const
        {
            return ConstIterator(0, NULL, NULL);
        }

        /// 
        /// Gets the hash for the given key.
        /// @param[in] key Key to get the hash.
        /// @return The hash for the given key.
        /// 
        uint GetKeyHash (Key key) const
        {
            return m_hash(key);
        }

        /// 
        /// Gets the total amount of memory used by the map in bytes.
        /// @return Memory used by the map in bytes.
        /// 
        size_t GetMemoryUsage () const
        {
            return sizeof(Node) * m_size + sizeof(Node*) * m_bucketCount;
        }

        /// 
        /// Gets the number of elements in the map.
        /// @return The number of elements in the map.
        /// 
        size_t GetSize () const
        {
            return m_size;
        }

        /// 
        /// Gets an iterator positioned at the first bucket of the map.
        /// @return An iterator positioned at the first bucket of the map.
        /// 
        Iterator GetStart ()
        {
            for (size_t i = 0; i < m_bucketCount; i++)
            {
                if (m_buckets[i] != NULL)
                {
                    return Iterator(i, m_buckets[i], NULL);
                }
            }

            return Iterator(0, NULL, NULL);
        }

        /// 
        /// Gets a const iterator positioned at the first bucket of the map.
        /// @return A const iterator positioned at the first bucket of the map.
        /// 
        ConstIterator GetStart () const
        {
            for (size_t i = 0; i < m_bucketCount; i++)
            {
                if (m_buckets[i] != NULL)
                {
                    return ConstIterator(i, m_buckets[i], NULL);
                }
            }

            return ConstIterator(0, NULL, NULL);
        }

        /// 
        /// Inserts a value in the map.
        /// @param[in] key Key to be associated with the value.
        /// @param[in] value Value being inserted.
        /// @return true if the value has been successfully inserted, false if a memory
        /// allocation error
        /// has occurred.
        /// @remarks If the map already contains a value with the specified key, it will
        /// be overwritten.
        /// 
        bool Insert (Key key, Value value)
        {
            Node* node;
            size_t index = _GetBucketIndexByKey(key);

            KeyEqualityComparator keyCompare;
            for (node = m_buckets[index]; node != NULL; node = node->next)
            {
                if (keyCompare(node->record.key, key))
                {
                    node->record.value = value;
                    return true;
                }
            }

            node = _CreateNode(key, value);

            if (node == NULL)
            {
                return false;
            }

            node->next = m_buckets[index];
            m_buckets[index] = node;
            m_size++;
            return true;
        }

        /// 
        /// Checks if the map is empty.
        /// @return true if the map is empty, false otherwise.
        /// 
        bool IsEmpty () const
        {
            return (GetSize() == 0);
        }

        /// 
        /// Checks if an iterator is at the end of the map.
        /// @param[in] it Iterator to check.
        /// @return true if the iterator is at the end of the map, false otherwise.
        /// 
        bool IsEnd (Iterator& it) const
        {
            return (it.m_node == NULL);
        }

        /// 
        /// Checks if a const iterator is at the end of the map.
        /// @param[in] it `const` iterator to check.
        /// @return true if the iterator is at the end of the map, false otherwise.
        /// 
        bool IsEnd (ConstIterator& it) const
        {
            return (it.m_node == NULL);
        }

        /// 
        /// Advances an iterator on the map.
        /// @param[in,out] it Pointer to the iterator to advance.
        /// 
        void Next (Iterator* it)
        {
            if (it->m_node->next != NULL)
            {
                it->m_prev = it->m_node;
                it->m_node = it->m_node->next;
                return;
            }

            for (size_t i = (it->m_index + 1); i < m_bucketCount; i++)
            {
                if (m_buckets[i] != NULL)
                {
                    it->m_index = i;
                    it->m_node = m_buckets[i];
                    it->m_prev = NULL;
                    return;
                }
            }

            it->m_node = NULL;
        }

        /// 
        /// Advances a const iterator on the map.
        /// @param[in,out] it Pointer to the const iterator to advance.
        /// 
        void Next (ConstIterator* it) const
        {
            if (it->m_node->next != NULL)
            {
                it->m_prev = it->m_node;
                it->m_node = it->m_node->next;
                return;
            }

            for (size_t i = (it->m_index + 1); i < m_bucketCount; i++)
            {
                if (m_buckets[i] != NULL)
                {
                    it->m_index = i;
                    it->m_node = m_buckets[i];
                    it->m_prev = NULL;
                    return;
                }
            }

            it->m_node = NULL;
        }

        /// 
        /// Removes a value associated with the specified key, if it exists.
        /// @param[in] key Key of the value being removed.
        /// 
        void Remove (Key key)
        {
            Iterator it;

            if (Find(key, &it))
            {
                return RemoveAt(&it);
            }
        }

        /// 
        /// Removes a value whose position is specified by an iterator.
        /// @param[in,out] it Pointer to an iterator that specifies the position of the
        /// value on the map.
        /// @remarks After the value is removed, the iterator is pointed to the next value.
        /// 
        void RemoveAt (Iterator* it)
        {
            Node* node = it->m_node;
            Node* next = node->next;

            if (it->m_prev != NULL)
            {
                it->m_prev->next = next;
                Next(it);
            }
            else
            {
                m_buckets[it->m_index] = next;
                Next(it);
                it->m_prev = NULL;
            }

            m_allocator.Release(node);
            m_size--;
        }

    protected:

        /// 
        /// Creates a node and initializes its record.
        /// @param[in] key Key of the record of the node.
        /// @param[in] value Value of the record of the node.
        /// @return The created node.
        /// 
        Node* _CreateNode (Key key, Value value)
        {
            return new(m_allocator) Node(key, value);
        }

        /// 
        /// Gets the bucket index of a hash.
        /// @param[in] keyHash Hash whose bucket index is to be retrieved.
        /// @return The bucket index of the hash.
        /// 
        size_t _GetBucketIndexByHash (uint keyHash) const
        {
            return SizePolicy::GetBucketIndex(keyHash, m_bucketCount);
        }

        /// 
        /// Gets the bucket index of a key.
        /// @param[in] key Key whose bucket index is to be retrieved.
        /// @return The bucket index of the key.
        /// 
        size_t _GetBucketIndexByKey (Key key) const
        {
            return SizePolicy::GetBucketIndex(m_hash(key), m_bucketCount);
        }

    protected:

        Allocator<Node> m_allocator;
        Node** m_buckets;
        HashFunction m_hash;
        size_t m_bucketCount;
        size_t m_size;

    private:

        Map (const Map& map);
        Map& operator= (const Map& map);
    };
}

#endif
