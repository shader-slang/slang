#ifndef SLANG_CORE_DICTIONARY_H
#define SLANG_CORE_DICTIONARY_H

#include "slang-common.h"
#include "slang-exception.h"
#include "slang-hash.h"
#include "slang-hashmap-impl.h"
#include "slang-linked-list.h"
#include "slang-list.h"
#include "slang-math.h"
#include "slang-uint-set.h"

#include <initializer_list>
#include <type_traits>
#include <utility>

namespace Slang
{
template<typename TKey, typename TValue>
class KeyValuePair
{
public:
    TKey key;
    TValue value;
    KeyValuePair() {}
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
    KeyValuePair(KeyValuePair<TKey, TValue>&& that) { operator=(_Move(that)); }
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
    HashCode getHashCode() const
    {
        return combineHash(Slang::getHashCode(key), Slang::getHashCode(value));
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

namespace KeyValueDetail
{

template<typename KEY, typename VALUE>
SLANG_FORCE_INLINE const KEY* getKey(const std::pair<KEY, VALUE>* in)
{
    return &in->first;
}
template<typename KEY, typename VALUE>
SLANG_FORCE_INLINE const KEY* getKey(const KeyValuePair<KEY, VALUE>* in)
{
    return &in->key;
}

template<typename KEY, typename VALUE>
SLANG_FORCE_INLINE const VALUE* getValue(const std::pair<KEY, VALUE>* in)
{
    return &in->second;
}
template<typename KEY, typename VALUE>
SLANG_FORCE_INLINE const VALUE* getValue(const KeyValuePair<KEY, VALUE>* in)
{
    return &in->value;
}

} // namespace KeyValueDetail

const float kMaxLoadFactor = 0.7f;

/// Declares `is_avalanching` only when the selected hash implementation avalanches.
///
/// `is_avalanching` is a member type, so it cannot be switched on with a constant the way
/// `kHasUniformHash` can; the declaration has to be inherited from a specialization instead.
/// This is the same shape as `DetectAvalanchingHash` in slang-hash.h, keyed directly on the
/// implementation rather than on a key type.
template<bool avalanching>
struct DeclareAvalanching
{
};
template<>
struct DeclareAvalanching<true>
{
    using is_avalanching = void;
};

/// Hashes any of the interchangeable text key types -- `UnownedStringSlice`,
/// `String`, `ImmutableHashedString` -- so that a dictionary keyed by one of
/// them can be probed with any of the others.
///
/// Without this, a `Dictionary<String, V>` probed with an `UnownedStringSlice`
/// silently converts the slice to a `String` first, because the backing map
/// only offers its heterogeneous `find` when both the hash and the comparator
/// declare `is_transparent`. That conversion heap-allocates and copies the
/// text on every lookup, hit or miss, and then throws the copy away.
///
/// The types opt in by declaring a member type `IsTextKey`, and in exchange
/// must satisfy two things: `getHashCode()` must agree across the family (it
/// does -- all three hash the same bytes with the same function), and
/// `getUnownedSlice()` must yield the text being keyed on. Anything without
/// those members is rejected at compile time rather than silently hashed some
/// other way; in particular a bare `const char*` is not a text key, because
/// hashing it would hash the pointer.
struct TextKeyHash : DeclareAvalanching<HashImpl::kIsAvalanching>
{
    using is_transparent = void;
    /// All the text key types hash their bytes with the selected hash
    /// function, so the result needs no further mixing by the map -- but only
    /// when that function avalanches, which is what the base class above
    /// checks. This matches the `kHasUniformHash` the types themselves declare.

    template<typename T, typename = typename T::IsTextKey>
    HashCode64 operator()(const T& key) const
    {
        return key.getHashCode();
    }
};

/// Compares any two of the interchangeable text key types; see `TextKeyHash`.
///
/// Both argument orders have to work, and which one a map uses is an
/// unspecified implementation detail, so this compares the two slices rather
/// than relying on a particular `operator==` overload existing between a
/// specific pair of the types.
struct TextKeyEqual
{
    using is_transparent = void;

    template<
        typename A,
        typename B,
        typename = typename A::IsTextKey,
        typename = typename B::IsTextKey>
    bool operator()(const A& a, const B& b) const
    {
        return a.getUnownedSlice() == b.getUnownedSlice();
    }
};

namespace DictionaryDetail
{
/// Selects the hash and comparator a `Dictionary` uses by default for `TKey`.
///
/// Key types that declare `IsTextKey` get the transparent pair above, so that
/// every `Dictionary<String, V>` in the codebase supports slice lookup without
/// having to be redeclared. Everything else keeps the previous defaults.
template<typename TKey, typename = void>
struct KeyTraits
{
    using Hash = Slang::Hash<TKey>;
    using KeyEqual = std::equal_to<TKey>;
};
template<typename TKey>
struct KeyTraits<TKey, std::void_t<typename TKey::IsTextKey>>
{
    using Hash = TextKeyHash;
    using KeyEqual = TextKeyEqual;
};
} // namespace DictionaryDetail

template<
    typename TKey,
    typename TValue,
    typename Hash = typename DictionaryDetail::KeyTraits<TKey>::Hash,
    typename KeyEqual = typename DictionaryDetail::KeyTraits<TKey>::KeyEqual>
class Dictionary
{
    // Which hash map actually backs this is a build-time choice; see
    // slang-hashmap-impl.h and the CMake option SLANG_HASHMAP.
    using InnerMap = HashMapImpl::Map<TKey, TValue, Hash, KeyEqual>;
    using ThisType = Dictionary<TKey, TValue, Hash, KeyEqual>;
    InnerMap map;

    // A caller may pass a value such as a string literal that can construct
    // TKey but is not a transparent probe type. In that case materialize one
    // TKey for the lookup; true text-key probes still avoid that allocation.
    template<typename K>
    auto _find(const K& key)
    {
        if constexpr (
            std::is_invocable_v<Hash, const K&> &&
            std::is_invocable_v<KeyEqual, const K&, const TKey&> &&
            std::is_invocable_v<KeyEqual, const TKey&, const K&>)
            return map.find(key);
        else
            return map.find(TKey(key));
    }

    template<typename K>
    auto _find(const K& key) const
    {
        if constexpr (
            std::is_invocable_v<Hash, const K&> &&
            std::is_invocable_v<KeyEqual, const K&, const TKey&> &&
            std::is_invocable_v<KeyEqual, const TKey&, const K&>)
            return map.find(key);
        else
            return map.find(TKey(key));
    }

public:
    Dictionary() = default;
    Dictionary(const Dictionary&) = default;
    Dictionary(Dictionary&&) = default;
    ThisType& operator=(const ThisType&) = default;
    ThisType& operator=(ThisType&&) = default;
    Dictionary(std::initializer_list<typename InnerMap::value_type> inits)
        : map(inits)
    {
    }

    //
    // Types
    //
    using Iterator = typename InnerMap::iterator;
    using ConstIterator = typename InnerMap::const_iterator;
    using KeyType = TKey;
    using ValueType = TValue;

    //
    // Iterators
    //

    // Iterating a non-const Dictionary yields a mutable mapped value, e.g.
    // `for (auto& [key, value] : dict) value.clear();`. That needs the
    // HashMapImpl::mutableIterator shim because tsl::robin_map's iterator
    // dereferences to a const pair; see its comment for the details.
    auto begin() { return HashMapImpl::mutableIterator(map.begin()); }
    auto begin() const { return map.begin(); }
    auto end() { return HashMapImpl::mutableIterator(map.end()); }
    auto end() const { return map.end(); }

    //
    // Modifiers
    //

    // Removes all values from the map
    void clear()
    {
        if (!map.empty())
            map.clear();
    }

    // Removes all values and releases backing storage.
    void clearAndDeallocate()
    {
        InnerMap emptyMap(0, map.hash_function(), map.key_eq(), map.get_allocator());
        map.swap(emptyMap);
    }

    // Erases the value at the specified key if it exists
    void remove(const TKey& key) { map.erase(key); }

    // Removes all values satifying the predicate:
    // bool predicate(pair<Key, Value>)
    template<typename Predicate>
    void removeIf(Predicate&& predicate)
    {
        // Iterates the backing map directly rather than through begin()/end(),
        // because eraseAndAdvance needs the map's own iterator type, and the
        // predicate only reads the entry.
        auto it = map.begin();
        while (it != map.end())
        {
            if (predicate(*it))
            {
                it = HashMapImpl::eraseAndAdvance(map, it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Reserves enough space for the specified number of values
    void reserve(Index size) { map.reserve(std::size_t(size)); };

    // Swap with another map
    void swapWith(ThisType& rhs) { std::swap(*this, rhs); }

    //
    // Query capacity
    //

    std::size_t getCount() const { return map.size(); }
    std::size_t getBucketCount() const { return map.bucket_count(); }

    //
    // Lookup
    //

    // Returns true if the map contains an equivalent key
    template<typename K>
    bool containsKey(const K& k) const
    {
        // Spelled with find() rather than contains() because std::unordered_map
        // only gained contains() in C++20 and we build as C++17.
        return _find(k) != map.end();
    }

    // Returns a valid pointer to the requested element, or nullptr if it
    // doesn't exist
    template<typename K>
    const TValue* tryGetValue(const K& key) const
    {
        auto i = _find(key);
        return i == map.end() ? nullptr : &(i->second);
    }
    // Returns a valid pointer to the requested element, or nullptr if it
    // doesn't exist
    template<typename K>
    TValue* tryGetValue(const K& key)
    {
        auto i = _find(key);
        return i == map.end() ? nullptr : std::addressof(HashMapImpl::valueOf(i));
    }

    // Returns true and copies the element into 'value' if present.
    // Otherwise returns false and value unmodified.
    template<typename K>
    bool tryGetValue(const K& key, TValue& value) const
    {
        auto i = _find(key);
        if (i == map.end())
            return false;
        value = i->second;
        return true;
    }

    // Returns a const reference to the value at the given key. Asserts if
    // the value doesn't exist
    const TValue& getValue(const TKey& key) const
    {
        if (const auto x = tryGetValue(key))
            return *x;
        SLANG_UNEXPECTED("The key does not exist in dictionary.");
    }

    // Returns a reference to the value at the given key. Asserts if the
    // value doesn't exist
    TValue& getValue(const TKey& key)
    {
        if (const auto x = tryGetValue(key))
            return *x;
        SLANG_UNEXPECTED("The key does not exist in dictionary.");
    }

    //
    // Combined Lookup and Insertion
    //

    // Tries to insert the given element, if a value was already present at
    // the given key then returns a pointer to that element instead.
    // Returns nullptr if insertion was successful.
    TValue* tryGetValueOrAdd(const typename InnerMap::value_type& kvPair)
    {
        const auto& [iterator, inserted] = map.insert(kvPair);
        return inserted ? nullptr : std::addressof(HashMapImpl::valueOf(iterator));
    }
    // Tries to insert the given element, if a value was already present at
    // the given key then returns a pointer to that element instead.
    // Returns nullptr if insertion was successful.
    TValue* tryGetValueOrAdd(typename InnerMap::value_type&& kvPair)
    {
        const auto& [iterator, inserted] = map.insert(std::move(kvPair));
        return inserted ? nullptr : std::addressof(HashMapImpl::valueOf(iterator));
    }
    /// Looks `key` up and, if it is absent, inserts an entry whose value is
    /// constructed in place from `args`. Returns a pointer to the mapped
    /// value -- found or freshly inserted -- together with whether an
    /// insertion took place.
    ///
    /// This costs a single hash and a single probe, and it does not construct
    /// the value at all when the key is already present. Prefer it to
    /// `tryGetValue` followed by `operator[]` or `add`, which hash and probe
    /// the same key twice, and to the pair-taking overloads below, which build
    /// a `value_type` before the map is consulted and so copy the key and the
    /// value even on a lookup that hits.
    ///
    /// For example, memoizing an expensive-to-build value reads as:
    ///
    ///     auto [entry, inserted] = cache.tryEmplace(key, nullptr);
    ///     if (inserted)
    ///         *entry = buildTheThing();
    ///     return *entry;
    ///
    template<typename... Args>
    std::pair<TValue*, bool> tryEmplace(const TKey& key, Args&&... args)
    {
        auto [iterator, inserted] = map.try_emplace(key, std::forward<Args>(args)...);
        return {std::addressof(iterator->second), inserted};
    }
    /// Overload of `tryEmplace` that moves the key when it has to be stored.
    template<typename... Args>
    std::pair<TValue*, bool> tryEmplace(TKey&& key, Args&&... args)
    {
        auto [iterator, inserted] = map.try_emplace(std::move(key), std::forward<Args>(args)...);
        return {std::addressof(iterator->second), inserted};
    }

    // Tries to insert the given element, if a value was already present at
    // the given key then returns a pointer to that element instead.
    // Returns nullptr if insertion was successful.
    TValue* tryGetValueOrAdd(const TKey& key, const TValue& value)
    {
        const auto [valuePtr, inserted] = tryEmplace(key, value);
        return inserted ? nullptr : valuePtr;
    }

    // Inserts the given value if it doesn't exist already
    // Return a reference to the (possibly new) value in the map
    TValue& getOrAddValue(const TKey& key, const TValue& defaultValue)
    {
        return *tryEmplace(key, defaultValue).first;
    }

    // Returns a reference to the value at the specified key, default
    // initializing it if it doesn't already exist
    TValue& operator[](const TKey& key) { return map[key]; }
    // Returns a reference to the value at the specified key, default
    // initializing it if it doesn't already exist
    TValue& operator[](TKey&& key) { return map[std::move(key)]; }

    //
    // Insertion
    //

    // Returns true if the value was inserted, returns false if the map
    // already has a value associated with this key
    bool addIfNotExists(typename InnerMap::value_type&& kvPair)
    {
        return !tryGetValueOrAdd(std::move(kvPair));
    }
    // Returns true if the value was inserted, returns false if the map
    // already has a value associated with this key
    bool addIfNotExists(const typename InnerMap::value_type& kvPair)
    {
        return !tryGetValueOrAdd(kvPair);
    }
    // Returns true if the value was inserted, returns false if the map
    // already has a value associated with this key
    bool addIfNotExists(const TKey& k, const TValue& v) { return tryEmplace(k, v).second; }
    // Returns true if the value was inserted, returns false if the map
    // already has a value associated with this key
    bool addIfNotExists(TKey&& k, TValue&& v)
    {
        return tryEmplace(std::move(k), std::move(v)).second;
    }

    // Asserts if the key already exists in the dictionary
    void add(typename InnerMap::value_type&& kvPair)
    {
        if (!addIfNotExists(std::move(kvPair)))
            SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
    }
    // Asserts if the key already exists in the dictionary
    void add(const typename InnerMap::value_type& kvPair)
    {
        if (!addIfNotExists(kvPair))
            SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
    }
    // Asserts if the key already exists in the dictionary
    void add(const TKey& key, const TValue& value)
    {
        if (!addIfNotExists(key, value))
            SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
    }
    // Asserts if the key already exists in the dictionary
    void add(TKey&& key, TValue&& value)
    {
        if (!addIfNotExists(std::move(key), std::move(value)))
            SLANG_ASSERT_FAILURE("The key already exists in Dictionary.");
    }

    // Inserts into the dictionary or assigns if the key already exists
    void set(const TKey& key, const TValue& value) { map.insert_or_assign(key, value); }
};

/* We may want to rename this, as strictly speaking _Caps names are reserved */
class _DummyClass
{
};

template<typename T, typename DictionaryType>
class HashSetBase
{
protected:
    DictionaryType dict;

private:
    void init() {} // Base case for recursion
    template<typename... Args>
    void init(const T& v, Args... args)
    {
        add(v);
        init(args...);
    }

public:
    HashSetBase() {}
    template<typename Arg, typename... Args>
    HashSetBase(Arg arg, Args... args)
    {
        init(arg, args...);
    }
    HashSetBase(const HashSetBase& set) { operator=(set); }
    HashSetBase(HashSetBase&& set) { operator=(_Move(set)); }
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
        typename DictionaryType::ConstIterator iter;

    public:
        Iterator() = default;
        const T& operator*() const { return *KeyValueDetail::getKey(std::addressof(*iter)); }
        const T* operator->() const { return KeyValueDetail::getKey(std::addressof(*iter)); }

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
        bool operator!=(const Iterator& that) const { return iter != that.iter; }
        bool operator==(const Iterator& that) const { return iter == that.iter; }
        Iterator(const typename DictionaryType::ConstIterator& _iter) { this->iter = _iter; }
    };
    Iterator begin() const { return Iterator(dict.begin()); }
    Iterator end() const { return Iterator(dict.end()); }

public:
    auto getCount() const { return dict.getCount(); }
    auto getBucketCount() const { return dict.getBucketCount(); }
    void clear() { dict.clear(); }
    void clearAndDeallocate() { dict.clearAndDeallocate(); }
    bool add(const T& obj) { return dict.addIfNotExists(obj, _DummyClass()); }
    bool add(T&& obj) { return dict.addIfNotExists(_Move(obj), _DummyClass()); }
    void remove(const T& obj) { dict.remove(obj); }
    bool contains(const T& obj) const { return dict.containsKey(obj); }
};
/// A set of `T`, backed by the selected implementation's own set type.
///
/// This used to be a `Dictionary<T, _DummyClass>`, which stores an empty value beside every key.
/// The value is not free: alignment rounds the entry up, so `HashSet<IRInst*>` took sixteen bytes
/// an entry where the key needs eight. Entry size decides how many entries share a cache line,
/// which is most of what a probe costs, and the compiler's hot sets are sets of pointers.
template<typename T, typename Hash = Slang::Hash<T>, typename KeyEqual = std::equal_to<T>>
class HashSet
{
private:
    using InnerSet = HashMapImpl::Set<T, Hash, KeyEqual>;
    InnerSet set;

    void init() {} // Base case for recursion
    template<typename... Args>
    void init(const T& v, Args... args)
    {
        add(v);
        init(args...);
    }

public:
    HashSet() = default;
    HashSet(const HashSet&) = default;
    HashSet(HashSet&&) = default;
    HashSet& operator=(const HashSet&) = default;
    HashSet& operator=(HashSet&&) = default;

    /// Construct from elements, as `HashSet<int> s(1, 2, 3)`.
    template<typename... Args>
    HashSet(const T& arg, Args... args)
    {
        init(arg, args...);
    }

    using Iterator = typename InnerSet::const_iterator;
    Iterator begin() const { return set.begin(); }
    Iterator end() const { return set.end(); }

    auto getCount() const { return set.size(); }
    auto getBucketCount() const { return set.bucket_count(); }

    void clear()
    {
        if (!set.empty())
            set.clear();
    }

    /// Empties the set and hands its memory back, where `clear` keeps it for reuse.
    void clearAndDeallocate()
    {
        InnerSet emptySet(0, set.hash_function(), set.key_eq(), set.get_allocator());
        set.swap(emptySet);
    }

    bool add(const T& obj) { return set.insert(obj).second; }
    bool add(T&& obj) { return set.insert(std::move(obj)).second; }
    void remove(const T& obj) { set.erase(obj); }
    bool contains(const T& obj) const { return set.find(obj) != set.end(); }
};

template<typename TKey, typename TValue>
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
    template<typename T>
    inline int getHashPos(T& key) const
    {
        const unsigned int hash = (unsigned int)getHashCode(key);
        return static_cast<int>((hash * 2654435761U) % m_bucketCountMinusOne);
    }
    template<typename T>
    FindPositionResult findPosition(const T& key) const
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
        SLANG_UNEXPECTED(
            "Hash map is full. This indicates an error in Key::Equal or Key::GetHashCode.");
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
            newDict.m_marks.resizeAndUnsetAll(newSize * 2);
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

        SLANG_UNEXPECTED("Inconsistent find result returned. This is a bug in OrderedDictionary "
                         "implementation.");
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

        SLANG_UNEXPECTED("Inconsistent find result returned. This is a bug in OrderedDictionary "
                         "implementation.");
    }

public:
    using Iterator = typename LinkedList<KeyValuePair<TKey, TValue>>::Iterator;
    using ConstIterator = typename LinkedList<KeyValuePair<TKey, TValue>>::ConstIterator;

    Iterator begin() { return m_kvPairs.begin(); }
    Iterator end() { return m_kvPairs.end(); }
    ConstIterator begin() const { return m_kvPairs.begin(); }
    ConstIterator end() const { return m_kvPairs.end(); }

public:
    void add(const TKey& key, const TValue& value) { add(KeyValuePair<TKey, TValue>(key, value)); }
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
        m_marks.resize(0);
    }
    template<typename T>
    bool containsKey(const T& key) const
    {
        if (m_bucketCountMinusOne == -1)
            return false;
        auto pos = findPosition(key);
        return pos.objectPosition != -1;
    }
    template<typename T>
    TValue* tryGetValue(const T& key) const
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
    template<typename T>
    bool tryGetValue(const T& key, TValue& value) const
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

            SLANG_UNEXPECTED("The key does not exist in dictionary.");
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
    template<typename... Args>
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
    template<typename Arg, typename... Args>
    OrderedDictionary(Arg arg, Args... args)
    {
        init(arg, args...);
    }
    OrderedDictionary(const OrderedDictionary<TKey, TValue>& other)
        : m_bucketCountMinusOne(-1), m_count(0), m_hashMap(0)
    {
        *this = other;
    }
    OrderedDictionary(OrderedDictionary<TKey, TValue>&& other)
        : m_bucketCountMinusOne(-1), m_count(0), m_hashMap(0)
    {
        *this = (_Move(other));
    }
    OrderedDictionary<TKey, TValue>& operator=(const OrderedDictionary<TKey, TValue>& other)
    {
        if (this == &other)
            return *this;
        clear();
        for (auto& item : other)
            add(item.key, item.value);
        return *this;
    }
    OrderedDictionary<TKey, TValue>& operator=(OrderedDictionary<TKey, TValue>&& other)
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

template<typename T>
class OrderedHashSet : public HashSetBase<T, OrderedDictionary<T, _DummyClass>>
{
public:
    T& getLast() { return this->dict.getLast().key; }
    void removeLast() { this->remove(getLast()); }
};
} // namespace Slang

#endif
