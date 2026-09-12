#ifndef SLANG_CORE_HASHMAP_IMPL_H
#define SLANG_CORE_HASHMAP_IMPL_H

//
// Selects the third-party hash map that backs `Slang::Dictionary` (and therefore
// `Slang::HashSet` and `Slang::ShortDictionary`).
//
// This exists so that the relative performance of the available hash maps can be
// measured on real compiler workloads: build the compiler several times with
// different values of the CMake option `SLANG_HASHMAP` and compare. Every
// implementation listed here is a flat or node-based open/closed addressing map
// with the same `Map<Key, Value, Hash, KeyEqual>` template signature, so the
// choice is a single type alias plus a couple of shims for the places where the
// libraries genuinely disagree (see `eraseAndAdvance` below).
//
// The hash *function* is chosen independently, by `slang-hash-impl.h` and the
// CMake option `SLANG_HASH`, so that map and hash can be varied as a cross
// product.
//

// The available implementations. `SLANG_HASHMAP_IMPL` is defined to one of these
// by the build system; it is not meant to be set per-file, because `Dictionary`
// appears in the layout of types shared across every translation unit.
#define SLANG_HASHMAP_UNORDERED_DENSE 1
#define SLANG_HASHMAP_BOOST_FLAT 2
#define SLANG_HASHMAP_BOOST_NODE 3
#define SLANG_HASHMAP_BOOST_UNORDERED 4
#define SLANG_HASHMAP_ABSL_FLAT 5
#define SLANG_HASHMAP_ABSL_NODE 6
#define SLANG_HASHMAP_TSL_ROBIN 7
#define SLANG_HASHMAP_STD 8

#ifndef SLANG_HASHMAP_IMPL
#define SLANG_HASHMAP_IMPL SLANG_HASHMAP_UNORDERED_DENSE
#endif

#if SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE
#include <ankerl/unordered_dense.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_FLAT
#include <boost/unordered/unordered_flat_map.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_NODE
#include <boost/unordered/unordered_node_map.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_UNORDERED
#include <boost/unordered/unordered_map.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_FLAT
#include <absl/container/flat_hash_map.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_NODE
#include <absl/container/node_hash_map.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_ROBIN
#include <tsl/robin_map.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_STD
#include <unordered_map>
#else
#error "SLANG_HASHMAP_IMPL is not set to one of the SLANG_HASHMAP_* values"
#endif

#include <iterator>
#include <type_traits>

namespace Slang
{
namespace HashMapImpl
{

#if SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = ankerl::unordered_dense::map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "ankerl::unordered_dense::map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_FLAT

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_flat_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_flat_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_NODE

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_node_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_node_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_UNORDERED

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_FLAT

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = absl::flat_hash_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "absl::flat_hash_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_NODE

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = absl::node_hash_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "absl::node_hash_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_ROBIN

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = tsl::robin_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "tsl::robin_map";

// Caveat: `tsl::robin_map`'s iterators are not conforming forward iterators.
// `[forward.iterators]` requires value-initialized iterators to compare equal to
// one another, but `robin_iterator`'s default constructor is written
// `robin_iterator() noexcept {}`, which leaves its `bucket_entry_ptr m_bucket`
// uninitialised, so two default-constructed iterators compare on garbage.
// Code that stores a map iterator and resets it to `{}` to mean "nowhere" works
// against every other backend here and silently misbehaves against this one.

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_STD

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = std::unordered_map<TKey, TValue, Hash, KeyEqual>;
constexpr const char* kName = "std::unordered_map";

#endif

/// Erases the entry `it` refers to and returns an iterator to the entry that
/// follows it, so that a loop can keep iterating after erasing.
///
/// This exists because the two things a caller would otherwise write are each
/// wrong for some of the implementations we support. `it = map.erase(it)` does
/// not compile against Abseil, whose `erase(iterator)` deliberately returns
/// `void` (computing the successor costs a scan that most callers do not want).
/// The classic node-based idiom `map.erase(it++)` is silently wrong for
/// `ankerl::unordered_dense::map`, which erases by moving the *last* element
/// into the erased slot -- the already-advanced iterator would then skip that
/// moved element and, at the end of the map, run past `end()`.
///
/// So: use the returned iterator when there is one, and otherwise take the
/// successor first and then erase. The latter is only sound because Abseil
/// invalidates just the erased element's own iterator, never its neighbours'.
template<typename TMap, typename TIterator>
TIterator eraseAndAdvance(TMap& map, TIterator it)
{
    if constexpr (std::is_void_v<decltype(map.erase(it))>)
    {
        TIterator next = std::next(it);
        map.erase(it);
        return next;
    }
    else
    {
        return map.erase(it);
    }
}

namespace Detail
{
template<typename TIterator, typename = void>
constexpr bool hasValueAccessor = false;
template<typename TIterator>
constexpr bool
    hasValueAccessor<TIterator, std::void_t<decltype(std::declval<TIterator&>().value())>> = true;
} // namespace Detail

/// Returns a mutable reference to the mapped value that `it` refers to.
///
/// Most of the implementations here let you write `it->second` and get a
/// mutable `TValue&` back, but `tsl::robin_map` deliberately hands out a
/// `const std::pair<Key, T>` through `operator*`/`operator->` so that the key
/// cannot be modified behind the map's back, and exposes the mutable mapped
/// value through a separate `value()` accessor instead. Spelling mutable value
/// access through this keeps that difference from leaking into `Dictionary`.
template<typename TIterator>
decltype(auto) valueOf(TIterator it)
{
    if constexpr (Detail::hasValueAccessor<TIterator>)
    {
        return it.value();
    }
    else
    {
        return (it->second);
    }
}

namespace Detail
{
/// True when dereferencing `TIterator` yields a `const` reference, i.e. the
/// mapped value cannot be written through it.
template<typename TIterator>
constexpr bool yieldsConstReference =
    std::is_const_v<std::remove_reference_t<decltype(*std::declval<const TIterator&>())>>;

/// Presents a `const`-dereferencing map iterator as one whose entry is mutable.
///
/// Only `tsl::robin_map` needs this; see `mutableIterator` below for why, and
/// for why casting the `const` away is sound.
template<typename TIterator>
class MutableValueIterator
{
    using Pair = std::remove_cv_t<typename std::iterator_traits<TIterator>::value_type>;

    TIterator m_inner;

public:
    using value_type = Pair;
    using reference = Pair&;
    using pointer = Pair*;
    using difference_type = typename std::iterator_traits<TIterator>::difference_type;
    using iterator_category = std::forward_iterator_tag;

    MutableValueIterator() = default;
    explicit MutableValueIterator(TIterator inner)
        : m_inner(inner)
    {
    }

    reference operator*() const { return const_cast<reference>(*m_inner); }
    pointer operator->() const { return std::addressof(**this); }

    MutableValueIterator& operator++()
    {
        ++m_inner;
        return *this;
    }
    MutableValueIterator operator++(int)
    {
        MutableValueIterator result = *this;
        ++m_inner;
        return result;
    }

    bool operator==(const MutableValueIterator& other) const { return m_inner == other.m_inner; }
    bool operator!=(const MutableValueIterator& other) const { return m_inner != other.m_inner; }
};
} // namespace Detail

/// Returns an iterator equivalent to `it` through which the mapped value can be
/// assigned to, so that `for (auto& [key, value] : dictionary)` can modify
/// `value` whichever implementation is selected.
///
/// Every implementation here except `tsl::robin_map` already dereferences to a
/// mutable entry, and for those this hands `it` straight back.
///
/// `tsl::robin_map` is the exception. It stores entries as `std::pair<Key, T>`
/// -- exactly as the default `ankerl::unordered_dense::map` backend does -- but
/// unlike ankerl it declares its iterator's `value_type` as *`const`*
/// `std::pair<Key, T>`, so that the key cannot be modified behind the map's
/// back, and offers the mutable mapped value only through a separate `value()`
/// accessor. For that one this wraps `it` in an iterator that casts the `const`
/// back off.
///
/// The cast is well defined rather than merely convenient: the entry it refers
/// to is a live element of the map's own bucket array, which is not a `const`
/// object. A non-`const` `tsl::robin_map` iterator holds a non-`const`
/// `bucket_iterator`, and the map itself writes through it (see
/// `robin_iterator::value()`, which returns `std::pair<Key, T>&` from the same
/// bucket). The `const` exists only on the iterator's declared `value_type`, as
/// tsl's chosen way of discouraging key mutation.
///
/// What the cast gives up is that discouragement: under this backend `key` in
/// the loop above is assignable, and assigning to it would leave the entry in
/// the wrong bucket. That is not a new hazard, because the default
/// `ankerl::unordered_dense::map` backend already exposes a mutable key the
/// same way; code that mutates a key is already broken on the default build.
template<typename TIterator>
auto mutableIterator(TIterator it)
{
    if constexpr (Detail::yieldsConstReference<TIterator>)
    {
        return Detail::MutableValueIterator<TIterator>(it);
    }
    else
    {
        return it;
    }
}

} // namespace HashMapImpl

/// Returns the name of the hash map implementation this build of Slang was
/// compiled against, e.g. "ankerl::unordered_dense::map". Use it to confirm
/// which binary you are measuring.
inline const char* getHashMapImplName()
{
    return HashMapImpl::kName;
}

} // namespace Slang

#endif
