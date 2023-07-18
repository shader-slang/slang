#ifndef SLANG_CORE_DICTIONARY_H
#define SLANG_CORE_DICTIONARY_H

#include "slang-list.h"
#include "slang-linked-list.h"
#include "slang-common.h"
#include "slang-uint-set.h"
#include "slang-exception.h"
#include "slang-math.h"
#include "slang-hash.h"

namespace Slang
{
    template<typename TKey, typename TValue>
    class KeyValuePair
    {
    public:
        TKey key;
        TValue value;
        KeyValuePair()
        {}
        KeyValuePair(const TKey& inKey, const TValue& inValue)
        {
            key = inKey;
            value = inValue;
        }
        KeyValuePair(TKey&& inKey, TValue&& inValue)
        {
            key = _Move(inKey);
            value = _Move(inValue);
        }
        KeyValuePair(TKey&& inKey, const TValue& inValue)
        {
            key = _Move(inKey);
            value = inValue;
        }
        KeyValuePair(const KeyValuePair<TKey, TValue>& that)
        {
            key = that.key;
            value = that.value;
        }
        KeyValuePair(KeyValuePair<TKey, TValue>&& that)
        {
            operator=(_Move(that));
        }
        KeyValuePair& operator=(KeyValuePair<TKey, TValue>&& that)
        {
            key = _Move(that.key);
            value = _Move(that.value);
            return *this;
        }
        KeyValuePair& operator=(const KeyValuePair<TKey, TValue>& that)
        {
            key = that.key;
            value = that.value;
            return *this;
        }
        HashCode getHashCode()
        {
            return combineHash(
                Slang::getHashCode(key),
                Slang::getHashCode(value));
        }
        bool operator==(const KeyValuePair<TKey, TValue>& that) const
        {
            return (key == that.key) && (value == that.value);
        }
    };

    template<typename TKey, typename TValue>
    inline KeyValuePair<TKey, TValue> KVPair(const TKey& k, const TValue& v)
    {
        return KeyValuePair<TKey, TValue>(k, v);
    }

    const float kMaxLoadFactor = 0.7f;

    template<typename TKey, typename TValue>
    class Dictionary
    {
        friend class Iterator;
        friend class ItemProxy;
    public:
        typedef TValue ValueType;
        typedef TKey KeyType;
        typedef Dictionary ThisType;
    private:
        inline int getProbeOffset(int /*probeId*/) const
        {
            // linear probing
            return 1;
        }
    private:
        int m_bucketCountMinusOne;
        int m_count;
        UIntSet m_marks;
        KeyValuePair<TKey, TValue>* m_hashMap;
        void deallocateAll()
        {
            if (m_hashMap)
                delete[] m_hashMap;
            m_hashMap = nullptr;
        }
        inline bool isDeleted(int pos) const
        {
            return m_marks.contains((pos << 1) + 1);
        }
        inline bool isEmpty(int pos) const
        {
            return !m_marks.contains((pos << 1));
        }
        inline void setDeleted(int pos, bool val)
        {
            if (val)
                m_marks.add((pos << 1) + 1);
            else
                m_marks.remove((pos << 1) + 1);
        }
        inline void setEmpty(int pos, bool val)
        {
            if (val)
                m_marks.remove((pos << 1));
            else
                m_marks.add((pos << 1));
        }
        struct FindPositionResult
        {
            int objectPosition;
            int insertionPosition;

            FindPositionResult()
            {
                objectPosition = -1;
                insertionPosition = -1;
            }
            FindPositionResult(int objPos, int insertPos)
            {
                objectPosition = objPos;
                insertionPosition = insertPos;
            }
        };
        template<typename KeyType>
        inline int getHashPos(KeyType& key) const
        {
            SLANG_ASSERT(m_bucketCountMinusOne > 0);
            const unsigned int hash = (unsigned int)getHashCode(key);
            return (hash * 2654435761u) % (unsigned int)(m_bucketCountMinusOne);
        }
        template<typename KeyType>
        FindPositionResult findPosition(const KeyType& key) const
        {
            int hashPos = getHashPos(const_cast<KeyType&>(key));
            int insertPos = -1;
            int numProbes = 0;
            while (numProbes <= m_bucketCountMinusOne)
            {
                if (isEmpty(hashPos))
                {
                    if (insertPos == -1)
                        return FindPositionResult(-1, hashPos);
                    else
                        return FindPositionResult(-1, insertPos);
                }
                else if (isDeleted(hashPos))
                {
                    if (insertPos == -1)
                        insertPos = hashPos;
                }
                else if (m_hashMap[hashPos].key == key)
                {
                    return FindPositionResult(hashPos, -1);
                }
                numProbes++;
                hashPos = (hashPos + getProbeOffset(numProbes)) & m_bucketCountMinusOne;
            }
            if (insertPos != -1)
                return FindPositionResult(-1, insertPos);
            SLANG_ASSERT_FAILURE("Hash map is full. This indicates an error in Key::Equal or Key::getHashCode.");
        }
        TValue& _insert(KeyValuePair<TKey, TValue>&& kvPair, int pos)
        {
            m_hashMap[pos] = _Move(kvPair);
            setEmpty(pos, false);
            setDeleted(pos, false);
            return m_hashMap[pos].value;
        }
        void maybeRehash()
        {
            if (m_bucketCountMinusOne == -1 || m_count >= int(kMaxLoadFactor * m_bucketCountMinusOne))
            {
                int newSize = (m_bucketCountMinusOne + 1) * 2;
                if (newSize == 0)
                {
                    newSize = 64;
                }
                reserve(newSize);
            }
        }

        bool addIfNotExists(KeyValuePair<TKey, TValue>&& kvPair)
        {
            maybeRehash();
            auto pos = findPosition(kvPair.key);
            if (pos.objectPosition != -1)
                return false;
            else if (pos.insertionPosition != -1)
            {
                m_count++;
                _insert(_Move(kvPair), pos.insertionPosition);
                return true;
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }
        void add(KeyValuePair<TKey, TValue>&& kvPair)
        {
            if (!addIfNotExists(_Move(kvPair)))
                SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
        }
        TValue& set(KeyValuePair<TKey, TValue>&& kvPair)
        {
            maybeRehash();
            auto pos = findPosition(kvPair.key);
            if (pos.objectPosition != -1)
                return _insert(_Move(kvPair), pos.objectPosition);
            else if (pos.insertionPosition != -1)
            {
                m_count++;
                return _insert(_Move(kvPair), pos.insertionPosition);
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }
    public:
        class Iterator
        {
        private:
            const Dictionary<TKey, TValue>* dict;
            int pos;
        public:
            KeyValuePair<TKey, TValue>& operator*() const
            {
                return dict->m_hashMap[pos];
            }
            KeyValuePair<TKey, TValue>* operator->() const
            {
                return dict->m_hashMap + pos;
            }
            Iterator& operator++()
            {
                if (pos > dict->m_bucketCountMinusOne)
                    return *this;
                pos++;
                while (pos <= dict->m_bucketCountMinusOne && (dict->isDeleted(pos) || dict->isEmpty(pos)))
                {
                    pos++;
                }
                return *this;
            }
            Iterator operator++(int)
            {
                Iterator rs = *this;
                operator++();
                return rs;
            }
            bool operator!=(const Iterator& that) const
            {
                return pos != that.pos || dict != that.dict;
            }
            bool operator==(const Iterator& that) const
            {
                return pos == that.pos && dict == that.dict;
            }
            Iterator(const Dictionary<TKey, TValue>* inDict, int inPos)
            {
                this->dict = inDict;
                this->pos = inPos;
            }
            Iterator()
            {
                this->dict = nullptr;
                this->pos = 0;
            }
        };

        Iterator begin() const
        {
            int pos = 0;
            while (pos < m_bucketCountMinusOne + 1)
            {
                if (isEmpty(pos) || isDeleted(pos))
                    pos++;
                else
                    break;
            }
            return Iterator(this, pos);
        }
        Iterator end() const
        {
            return Iterator(this, m_bucketCountMinusOne + 1);
        }
    public:
        void add(const TKey& key, const TValue& value)
        {
            add(KeyValuePair<TKey, TValue>(key, value));
        }
        void add(TKey&& key, TValue&& value)
        {
            add(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
        }
        bool addIfNotExists(const TKey& key, const TValue& value)
        {
            return addIfNotExists(KeyValuePair<TKey, TValue>(key, value));
        }
        bool addIfNotExists(TKey&& key, TValue&& value)
        {
            return addIfNotExists(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
        }
        void remove(const TKey& key)
        {
            if (m_count == 0)
                return;
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                setDeleted(pos.objectPosition, true);
                m_count--;
            }
        }
        void clear()
        {
            m_count = 0;
            m_marks.clear();
        }

        void reserve(int newSize)
        {
            if (newSize <= m_bucketCountMinusOne + 1)
                return;

            Dictionary<TKey, TValue> newDict;
            newDict.m_bucketCountMinusOne = newSize - 1;
            newDict.m_hashMap = new KeyValuePair<TKey, TValue>[newSize];
            newDict.m_marks.resizeAndClear(newSize * 2);
            if (m_hashMap)
            {
                for (auto& kvPair : *this)
                {
                    newDict.add(_Move(kvPair));
                }
            }
            *this = _Move(newDict);
        }

        TValue* tryGetValueOrAdd(const TKey& key, const TValue& value)
        {
            maybeRehash();
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                return &m_hashMap[pos.objectPosition].value;
            }
            else if (pos.insertionPosition != -1)
            {
                // Make pair
                KeyValuePair<TKey, TValue> kvPair(_Move(key), _Move(value));
                m_count++;
                _insert(_Move(kvPair), pos.insertionPosition);
                return nullptr;
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }

            /// This differs from tryGetValueOrAdd, in that it always returns the Value held in the Dictionary.
            /// If there isn't already an entry for 'key', a value is added with defaultValue. 
        TValue& getOrAddValue(const TKey& key, const TValue& defaultValue)
        {
            maybeRehash();
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                return m_hashMap[pos.objectPosition].value;
            }
            else if (pos.insertionPosition != -1)
            {
                // Make pair
                KeyValuePair<TKey, TValue> kvPair(_Move(key), _Move(defaultValue));
                m_count++;
                return _insert(_Move(kvPair), pos.insertionPosition);
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }
        void set(const TKey& key, const TValue& value)
        {
            if (auto ptr = tryGetValueOrAdd(key, value))
            {
                *ptr = value;
            }
        }

        template<typename KeyType>
        bool containsKey(const KeyType& key) const
        {
            if (m_bucketCountMinusOne == -1)
                return false;
            auto pos = findPosition(key);
            return pos.objectPosition != -1;
        }
        template<typename KeyType>
        bool tryGetValue(const KeyType& key, TValue& value) const
        {
            if (m_bucketCountMinusOne == -1)
                return false;
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                value = m_hashMap[pos.objectPosition].value;
                return true;
            }
            return false;
        }
        template<typename KeyType>
        TValue* tryGetValue(const KeyType& key) const
        {
            if (m_bucketCountMinusOne == -1)
                return nullptr;
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                return &m_hashMap[pos.objectPosition].value;
            }
            return nullptr;
        }

        class ItemProxy
        {
        private:
            const Dictionary<TKey, TValue>* dict;
            TKey key;
        public:
            ItemProxy(const TKey& _key, const Dictionary<TKey, TValue>* _dict)
            {
                this->dict = _dict;
                this->key = _key;
            }
            ItemProxy(TKey&& _key, const Dictionary<TKey, TValue>* _dict)
            {
                this->dict = _dict;
                this->key = _Move(_key);
            }
            TValue& getValue() const
            {
                auto pos = dict->findPosition(key);
                if (pos.objectPosition != -1)
                {
                    return dict->m_hashMap[pos.objectPosition].value;
                }
                else
                    SLANG_ASSERT_FAILURE("The key does not exist in dictionary.");
            }
            inline TValue& operator()() const
            {
                return getValue();
            }
            operator TValue&() const
            {
                return getValue();
            }
            TValue& operator=(const TValue& val) const
            {
                return ((Dictionary<TKey, TValue>*)dict)->set(KeyValuePair<TKey, TValue>(_Move(key), val));
            }
            TValue& operator=(TValue&& val) const
            {
                return ((Dictionary<TKey, TValue>*)dict)->set(KeyValuePair<TKey, TValue>(_Move(key), _Move(val)));
            }
        };
        ItemProxy operator[](const TKey& key) const
        {
            return ItemProxy(key, this);
        }
        ItemProxy operator[](TKey&& key) const
        {
            return ItemProxy(_Move(key), this);
        }
        int getCount() const
        {
            return m_count;
        }

            /// Swap this with rhs
        void swapWith(ThisType& rhs);

    private:
        template<typename... Args>
        void init(const KeyValuePair<TKey, TValue>& kvPair, Args... args)
        {
            add(kvPair);
            init(args...);
        }
    public:
        Dictionary()
        {
            m_bucketCountMinusOne = -1;
            m_count = 0;
            m_hashMap = nullptr;
        }
        template<typename Arg, typename... Args>
        Dictionary(Arg arg, Args... args)
        {
            init(arg, args...);
        }
        Dictionary(const Dictionary<TKey, TValue>& other)
            : m_bucketCountMinusOne(-1), m_count(0), m_hashMap(nullptr)
        {
            *this = other;
        }
        Dictionary(Dictionary<TKey, TValue>&& other)
            : m_bucketCountMinusOne(-1), m_count(0), m_hashMap(nullptr)
        {
            *this = (_Move(other));
        }
        Dictionary<TKey, TValue>& operator=(const Dictionary<TKey, TValue>& other)
        {
            if (this == &other)
                return *this;
            deallocateAll();
            m_bucketCountMinusOne = other.m_bucketCountMinusOne;
            m_count = other.m_count;
            m_hashMap = new KeyValuePair<TKey, TValue>[other.m_bucketCountMinusOne + 1];
            m_marks = other.m_marks;
            for (int i = 0; i <= m_bucketCountMinusOne; i++)
                m_hashMap[i] = other.m_hashMap[i];
            return *this;
        }
        Dictionary<TKey, TValue>& operator=(Dictionary<TKey, TValue>&& other)
        {
            if (this == &other)
                return *this;
            deallocateAll();
            m_bucketCountMinusOne = other.m_bucketCountMinusOne;
            m_count = other.m_count;
            m_hashMap = other.m_hashMap;
            m_marks = _Move(other.m_marks);
            other.m_hashMap = nullptr;
            other.m_count = 0;
            other.m_bucketCountMinusOne = -1;
            return *this;
        }
        ~Dictionary()
        {
            deallocateAll();
        }
    };

    // ---------------------------------------------------------
    template<typename TKey, typename TValue>
    void Dictionary<TKey, TValue>::swapWith(ThisType& rhs)
    {
        Swap(m_bucketCountMinusOne, rhs.m_bucketCountMinusOne);
        Swap(m_count, rhs.m_count);
        m_marks.swapWith(rhs.m_marks);
        Swap(m_hashMap, rhs.m_hashMap);
    }

    /* We may want to rename this, as strictly speaking _Caps names are reserved */
    class _DummyClass
    {};

    template<typename T, typename DictionaryType>
    class HashSetBase
    {
    protected:
        DictionaryType dict;
    private:
        template<typename... Args>
        void init(const T& v, Args... args)
        {
            add(v);
            init(args...);
        }
    public:
        HashSetBase()
        {}
        template<typename Arg, typename... Args>
        HashSetBase(Arg arg, Args... args)
        {
            init(arg, args...);
        }
        HashSetBase(const HashSetBase& set)
        {
            operator=(set);
        }
        HashSetBase(HashSetBase&& set)
        {
            operator=(_Move(set));
        }
        HashSetBase& operator=(const HashSetBase& set)
        {
            dict = set.dict;
            return *this;
        }
        HashSetBase& operator=(HashSetBase&& set)
        {
            dict = _Move(set.dict);
            return *this;
        }
    public:
        class Iterator
        {
        private:
            typename DictionaryType::Iterator iter;
        public:
            Iterator() = default;
            T& operator*() const
            {
                return (*iter).key;
            }
            T* operator->() const
            {
                return &(*iter).key;
            }
            Iterator& operator++()
            {
                ++iter;
                return *this;
            }
            Iterator operator++(int)
            {
                Iterator rs = *this;
                operator++();
                return rs;
            }
            bool operator!=(const Iterator& that) const
            {
                return iter != that.iter;
            }
            bool operator==(const Iterator& that) const
            {
                return iter == that.iter;
            }
            Iterator(const typename DictionaryType::Iterator& _iter)
            {
                this->iter = _iter;
            }
        };
        Iterator begin() const
        {
            return Iterator(dict.begin());
        }
        Iterator end() const
        {
            return Iterator(dict.end());
        }
    public:
        int getCount() const
        {
            return dict.getCount();
        }
        void clear()
        {
            dict.clear();
        }
        bool add(const T& obj)
        {
            return dict.addIfNotExists(obj, _DummyClass());
        }
        bool add(T&& obj)
        {
            return dict.addIfNotExists(_Move(obj), _DummyClass());
        }
        void remove(const T& obj)
        {
            dict.remove(obj);
        }
        bool contains(const T& obj) const
        {
            return dict.containsKey(obj);
        }
    };
    template <typename T>
    class HashSet : public HashSetBase<T, Dictionary<T, _DummyClass>>
    {};

    template <typename TKey, typename TValue>
    class OrderedDictionary
    {
        friend class Iterator;
        friend class ItemProxy;

    private:
        inline int getProbeOffset(int /*probeIdx*/) const
        {
            // quadratic probing
            return 1;
        }

    private:
        int m_bucketCountMinusOne;
        int m_count;
        UIntSet m_marks;

        LinkedList<KeyValuePair<TKey, TValue>> m_kvPairs;
        LinkedNode<KeyValuePair<TKey, TValue>>** m_hashMap;
        void deallocateAll()
        {
            if (m_hashMap)
                delete[] m_hashMap;
            m_hashMap = nullptr;
            m_kvPairs.clear();
        }
        inline bool isDeleted(int pos) const { return m_marks.contains((pos << 1) + 1); }
        inline bool isEmpty(int pos) const { return !m_marks.contains((pos << 1)); }
        inline void setDeleted(int pos, bool val)
        {
            if (val)
                m_marks.add((pos << 1) + 1);
            else
                m_marks.remove((pos << 1) + 1);
        }
        inline void setEmpty(int pos, bool val)
        {
            if (val)
                m_marks.remove((pos << 1));
            else
                m_marks.add((pos << 1));
        }
        struct FindPositionResult
        {
            int objectPosition;
            int insertionPosition;
            FindPositionResult()
            {
                objectPosition = -1;
                insertionPosition = -1;
            }
            FindPositionResult(int objPos, int insertPos)
            {
                objectPosition = objPos;
                insertionPosition = insertPos;
            }
        };
        template <typename T> inline int getHashPos(T& key) const
        {
            const unsigned int hash = (unsigned int)getHashCode(key);
            return ((unsigned int)(hash * 2654435761)) % m_bucketCountMinusOne;
        }
        template <typename T> FindPositionResult findPosition(const T& key) const
        {
            int hashPos = getHashPos((T&)key);
            int insertPos = -1;
            int numProbes = 0;
            while (numProbes <= m_bucketCountMinusOne)
            {
                if (isEmpty(hashPos))
                {
                    if (insertPos == -1)
                        return FindPositionResult(-1, hashPos);
                    else
                        return FindPositionResult(-1, insertPos);
                }
                else if (isDeleted(hashPos))
                {
                    if (insertPos == -1)
                        insertPos = hashPos;
                }
                else if (m_hashMap[hashPos]->value.key == key)
                {
                    return FindPositionResult(hashPos, -1);
                }
                numProbes++;
                hashPos = (hashPos + getProbeOffset(numProbes)) & m_bucketCountMinusOne;
            }
            if (insertPos != -1)
                return FindPositionResult(-1, insertPos);
            SLANG_ASSERT_FAILURE("Hash map is full. This indicates an error in Key::Equal or Key::GetHashCode.");
        }
        TValue& _insert(KeyValuePair<TKey, TValue>&& kvPair, int pos)
        {
            auto node = m_kvPairs.addLast();
            node->value = _Move(kvPair);
            m_hashMap[pos] = node;
            setEmpty(pos, false);
            setDeleted(pos, false);
            return node->value.value;
        }
        void maybeRehash()
        {
            if (m_bucketCountMinusOne == -1 || m_count / (float)m_bucketCountMinusOne >= kMaxLoadFactor)
            {
                int newSize = (m_bucketCountMinusOne + 1) * 2;
                if (newSize == 0)
                {
                    newSize = 128;
                }
                OrderedDictionary<TKey, TValue> newDict;
                newDict.m_bucketCountMinusOne = newSize - 1;
                newDict.m_hashMap = new LinkedNode<KeyValuePair<TKey, TValue>>*[newSize];
                newDict.m_marks.resizeAndClear(newSize * 2);
                if (m_hashMap)
                {
                    for (auto& kvPair : *this)
                    {
                        newDict.add(_Move(kvPair));
                    }
                }
                *this = _Move(newDict);
            }
        }

        bool addIfNotExists(KeyValuePair<TKey, TValue>&& kvPair)
        {
            maybeRehash();
            auto pos = findPosition(kvPair.key);
            if (pos.objectPosition != -1)
                return false;
            else if (pos.insertionPosition != -1)
            {
                m_count++;
                _insert(_Move(kvPair), pos.insertionPosition);
                return true;
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }
        void add(KeyValuePair<TKey, TValue>&& kvPair)
        {
            if (!addIfNotExists(_Move(kvPair)))
                SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
        }
        TValue& set(KeyValuePair<TKey, TValue>&& kvPair)
        {
            maybeRehash();
            auto pos = findPosition(kvPair.key);
            if (pos.objectPosition != -1)
            {
                m_hashMap[pos.objectPosition]->removeAndDelete();
                return _insert(_Move(kvPair), pos.objectPosition);
            }
            else if (pos.insertionPosition != -1)
            {
                m_count++;
                return _insert(_Move(kvPair), pos.insertionPosition);
            }
            else
                SLANG_ASSERT_FAILURE("Inconsistent find result returned. This is a bug in Dictionary implementation.");
        }

    public:
        typedef typename LinkedList<KeyValuePair<TKey, TValue>>::Iterator Iterator;

        typename LinkedList<KeyValuePair<TKey, TValue>>::Iterator begin() const
        {
            return m_kvPairs.begin();
        }
        typename LinkedList<KeyValuePair<TKey, TValue>>::Iterator end() const
        {
            return m_kvPairs.end();
        }

    public:
        void add(const TKey& key, const TValue& value)
        {
            add(KeyValuePair<TKey, TValue>(key, value));
        }
        void add(TKey&& key, TValue&& value)
        {
            add(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
        }
        bool addIfNotExists(const TKey& key, const TValue& value)
        {
            return addIfNotExists(KeyValuePair<TKey, TValue>(key, value));
        }
        bool addIfNotExists(TKey&& key, TValue&& value)
        {
            return addIfNotExists(KeyValuePair<TKey, TValue>(_Move(key), _Move(value)));
        }
        void remove(const TKey& key)
        {
            if (m_count > 0)
            {
                auto pos = findPosition(key);
                if (pos.objectPosition != -1)
                {
                    m_kvPairs.removeAndDelete(m_hashMap[pos.objectPosition]);
                    m_hashMap[pos.objectPosition] = 0;
                    setDeleted(pos.objectPosition, true);
                    m_count--;
                }
            }
        }
        void clear()
        {
            m_count = 0;
            m_kvPairs.clear();
            m_marks.clear();
        }
        template <typename T> bool containsKey(const T& key) const
        {
            if (m_bucketCountMinusOne == -1)
                return false;
            auto pos = findPosition(key);
            return pos.objectPosition != -1;
        }
        template <typename T> TValue* tryGetValue(const T& key) const
        {
            if (m_bucketCountMinusOne == -1)
                return nullptr;
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                return &(m_hashMap[pos.objectPosition]->value.value);
            }
            return nullptr;
        }
        template <typename T> bool tryGetValue(const T& key, TValue& value) const
        {
            if (m_bucketCountMinusOne == -1)
                return false;
            auto pos = findPosition(key);
            if (pos.objectPosition != -1)
            {
                value = m_hashMap[pos.objectPosition]->value.value;
                return true;
            }
            return false;
        }
        class ItemProxy
        {
        private:
            const OrderedDictionary<TKey, TValue>* dict;
            TKey key;

        public:
            ItemProxy(const TKey& _key, const OrderedDictionary<TKey, TValue>* _dict)
            {
                this->dict = _dict;
                this->key = _key;
            }
            ItemProxy(TKey&& _key, const OrderedDictionary<TKey, TValue>* _dict)
            {
                this->dict = _dict;
                this->key = _Move(_key);
            }
            TValue& getValue() const
            {
                auto pos = dict->findPosition(key);
                if (pos.objectPosition != -1)
                {
                    return dict->m_hashMap[pos.objectPosition]->value.value;
                }
                else
                {
                    SLANG_ASSERT_FAILURE("The key does not exists in dictionary.");
                }
            }
            inline TValue& operator()() const { return getValue(); }
            operator TValue&() const { return getValue(); }
            TValue& operator=(const TValue& val)
            {
                return ((OrderedDictionary<TKey, TValue>*)dict)
                    ->set(KeyValuePair<TKey, TValue>(_Move(key), val));
            }
            TValue& operator=(TValue&& val)
            {
                return ((OrderedDictionary<TKey, TValue>*)dict)
                    ->set(KeyValuePair<TKey, TValue>(_Move(key), _Move(val)));
            }
        };
        ItemProxy operator[](const TKey& key) const { return ItemProxy(key, this); }
        ItemProxy operator[](TKey&& key) const { return ItemProxy(_Move(key), this); }

        int getCount() const { return m_count; }
        KeyValuePair<TKey, TValue>& getFirst() const { return m_kvPairs.getFirst(); }
        KeyValuePair<TKey, TValue>& getLast() const { return m_kvPairs.getLast(); }

    private:
        template <typename... Args>
        void init(const KeyValuePair<TKey, TValue>& kvPair, Args... args)
        {
            add(kvPair);
            init(args...);
        }

    public:
        OrderedDictionary()
        {
            m_bucketCountMinusOne = -1;
            m_count = 0;
            m_hashMap = 0;
        }
        template <typename Arg, typename... Args> OrderedDictionary(Arg arg, Args... args)
        {
            init(arg, args...);
        }
        OrderedDictionary(const OrderedDictionary<TKey, TValue>& other)
            : m_bucketCountMinusOne(-1)
            , m_count(0)
            , m_hashMap(0)
        {
            *this = other;
        }
        OrderedDictionary(OrderedDictionary<TKey, TValue>&& other)
            : m_bucketCountMinusOne(-1)
            , m_count(0)
            , m_hashMap(0)
        {
            *this = (_Move(other));
        }
        OrderedDictionary<TKey, TValue>&
        operator=(const OrderedDictionary<TKey, TValue>& other)
        {
            if (this == &other)
                return *this;
            clear();
            for (auto& item : other)
                add(item.key, item.value);
            return *this;
        }
        OrderedDictionary<TKey, TValue>&
        operator=(OrderedDictionary<TKey, TValue>&& other)
        {
            if (this == &other)
                return *this;
            deallocateAll();
            m_bucketCountMinusOne = other.m_bucketCountMinusOne;
            m_count = other.m_count;
            m_hashMap = other.m_hashMap;
            m_marks = _Move(other.m_marks);
            other.m_hashMap = 0;
            other.m_count = 0;
            other.m_bucketCountMinusOne = -1;
            m_kvPairs = _Move(other.m_kvPairs);
            return *this;
        }
        ~OrderedDictionary() { deallocateAll(); }
    };

    template <typename T> class OrderedHashSet : public HashSetBase<T, OrderedDictionary<T, _DummyClass>>
    {
    public:
        T& getLast()
        {
            return this->dict.getLast().key;
        }
        void removeLast()
        {
            this->remove(getLast());
        }
    };
}

#endif
