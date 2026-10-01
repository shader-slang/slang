#ifndef SLANG_CORE_HASHMAP_IMPL_H
#define SLANG_CORE_HASHMAP_IMPL_H

//
// Selects the third-party hash map that backs `Slang::Dictionary` (and therefore
// `Slang::HashSet` and `Slang::ShortDictionary`).
//
// This exists so that the relative performance of the available hash maps can be
// measured on real compiler workloads: build the compiler several times with
// different values of the CMake option `SLANG_HASHMAP` and compare. The set is
// chosen to span the designs rather than to be exhaustive: bucket chaining
// (`STD`, `BOOST_UNORDERED`), SIMD-metadata open addressing flat and node-based
// (`ABSL_*`, `BOOST_FLAT`/`BOOST_NODE`, `GTL_FLAT`), robin-hood
// (`TSL_ROBIN`), hopscotch (`TSL_HOPSCOTCH`), sparse-group storage
// (`TSL_SPARSE`), and an index array over a dense value vector
// (`UNORDERED_DENSE`, `UNORDERED_DENSE_SEGMENTED`). All of them present the
// same `Map<Key, Value, Hash, KeyEqual>` template signature, so the choice is a
// single type alias plus a couple of shims for the places where the libraries
// genuinely disagree (see `eraseAndAdvance` below).
//
// The hash *function* is chosen independently, by `slang-hash-impl.h` and the
// CMake option `SLANG_HASH`, so that map and hash can be varied as a cross
// product.
//

// The available implementations. `SLANG_HASHMAP_IMPL` is defined to one of these
// by the build system; it is not meant to be set per-file, because `Dictionary`
// appears in the layout of types shared across every translation unit.
#define SLANG_HASHMAP_UNORDERED_DENSE 1
#define SLANG_HASHMAP_UNORDERED_DENSE_SEGMENTED 2
#define SLANG_HASHMAP_BOOST_FLAT 3
#define SLANG_HASHMAP_BOOST_NODE 4
#define SLANG_HASHMAP_BOOST_UNORDERED 5
#define SLANG_HASHMAP_ABSL_FLAT 6
#define SLANG_HASHMAP_ABSL_NODE 7
#define SLANG_HASHMAP_GTL_FLAT 8
#define SLANG_HASHMAP_TSL_ROBIN 9
#define SLANG_HASHMAP_TSL_HOPSCOTCH 10
#define SLANG_HASHMAP_TSL_SPARSE 11
#define SLANG_HASHMAP_STD 12

#ifndef SLANG_HASHMAP_IMPL
#define SLANG_HASHMAP_IMPL SLANG_HASHMAP_UNORDERED_DENSE
#endif

#if SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE || \
    SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE_SEGMENTED
#include <ankerl/unordered_dense.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_FLAT
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_NODE
#include <boost/unordered/unordered_node_map.hpp>
#include <boost/unordered/unordered_node_set.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_UNORDERED
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_FLAT
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_NODE
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_GTL_FLAT
#include <gtl/phmap.hpp>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_ROBIN
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_HOPSCOTCH
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_SPARSE
#include <tsl/sparse_map.h>
#include <tsl/sparse_set.h>
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_STD
#include <unordered_map>
#include <unordered_set>
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
template<typename TKey, typename Hash, typename KeyEqual>
using Set = ankerl::unordered_dense::set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "ankerl::unordered_dense::map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE_SEGMENTED

// The same design as `UNORDERED_DENSE` -- a bucket array of indices into a
// vector holding the entries themselves -- except that the entry vector is a
// `segmented_vector`, a list of fixed-size blocks rather than one allocation.
// Growing therefore appends a block instead of reallocating and moving every
// entry, which bounds the latency spike when one of the long-lived
// session-scope dictionaries doubles, at the cost of an extra indirection on
// every access.
template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = ankerl::unordered_dense::segmented_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = ankerl::unordered_dense::segmented_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "ankerl::unordered_dense::segmented_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_FLAT

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_flat_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = boost::unordered_flat_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_flat_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_NODE

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_node_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = boost::unordered_node_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_node_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_UNORDERED

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = boost::unordered_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = boost::unordered_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "boost::unordered_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_FLAT

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = absl::flat_hash_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = absl::flat_hash_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "absl::flat_hash_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_ABSL_NODE

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = absl::node_hash_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = absl::node_hash_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "absl::node_hash_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_GTL_FLAT

// A reimplementation of Abseil's swisstable that does not depend on the rest of
// Abseil. Worth measuring separately from `ABSL_FLAT` despite the shared
// design, because it differs in the two respects that decide whether it could
// become the default: it is header-only, so it does not add Abseil to the
// build, and it does not seed itself per process, so unlike `SLANG_HASH=ABSL`
// its hash values may escape into a serialized module.
template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = gtl::flat_hash_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = gtl::flat_hash_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "gtl::flat_hash_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_ROBIN

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = tsl::robin_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = tsl::robin_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "tsl::robin_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_HOPSCOTCH

// Hopscotch hashing: every key is stored within a fixed-size neighbourhood of
// its home bucket, and an insertion that finds the neighbourhood full displaces
// an existing entry towards its own home rather than probing onwards. Lookups
// therefore touch a bounded, contiguous run of buckets. This is the only
// backend here using that strategy.
template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = tsl::hopscotch_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = tsl::hopscotch_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "tsl::hopscotch_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_TSL_SPARSE

// Sparse-group storage: buckets are held in groups of 64 with an occupancy
// bitmap per group, and only the occupied slots are allocated. That trades
// lookup speed for a much smaller footprint, so this is the backend to measure
// when the question is peak memory rather than wall clock.
template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = tsl::sparse_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = tsl::sparse_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "tsl::sparse_map";

#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_STD

template<typename TKey, typename TValue, typename Hash, typename KeyEqual>
using Map = std::unordered_map<TKey, TValue, Hash, KeyEqual>;
template<typename TKey, typename Hash, typename KeyEqual>
using Set = std::unordered_set<TKey, Hash, KeyEqual>;
constexpr const char* kName = "std::unordered_map";

#endif

// Caveat, for all three of the tsl maps: their iterators are not conforming
// forward iterators. `[forward.iterators]` requires value-initialized iterators
// to compare equal to one another, but each of `robin_iterator`,
// `hopscotch_iterator` and `sparse_iterator` has a default constructor written
// `noexcept {}`, which leaves the bucket pointer it compares on uninitialised.
// Code that stores a map iterator and resets it to `{}` to mean "nowhere" works
// against every other backend here and silently misbehaves against these.

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
/// mutable `TValue&` back, but the tsl maps deliberately hand out a
/// `const std::pair<Key, T>` through `operator*`/`operator->` so that the key
/// cannot be modified behind the map's back, and expose the mutable mapped
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
/// Only the tsl maps need this; see `mutableIterator` below for why, and for
/// why casting the `const` away is sound.
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
/// Every implementation here except the tsl maps already dereferences to a
/// mutable entry, and for those this hands `it` straight back.
///
/// `tsl::robin_map`, `tsl::hopscotch_map` and `tsl::sparse_map` are the
/// exceptions. They store entries as `std::pair<Key, T>` -- exactly as the
/// default `ankerl::unordered_dense::map` backend does -- but unlike ankerl
/// they declare their iterator's `value_type` as *`const`*
/// `std::pair<Key, T>`, so that the key cannot be modified behind the map's
/// back, and offer the mutable mapped value only through a separate `value()`
/// accessor. For those this wraps `it` in an iterator that casts the `const`
/// back off.
///
/// The cast is well defined rather than merely convenient: the entry it refers
/// to is a live element of the map's own bucket array, which is not a `const`
/// object. A non-`const` tsl iterator holds a non-`const` `bucket_iterator`,
/// and the map itself writes through it (see `robin_iterator::value()`, which
/// returns `std::pair<Key, T>&` from the same bucket). The `const` exists only
/// on the iterator's declared `value_type`, as tsl's chosen way of discouraging
/// key mutation.
///
/// What the cast gives up is that discouragement: under these backends `key` in
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
