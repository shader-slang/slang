#ifndef SLANG_CORE_HASH_IMPL_H
#define SLANG_CORE_HASH_IMPL_H

//
// Selects the third-party hash *function* that `Slang::Hash<T>` falls back to for
// types that do not define their own `getHashCode()` -- integers, enums,
// pointers, and the standard string types.
//
// This is deliberately independent of `slang-hashmap-impl.h`, which selects the
// hash *map*. The quality of the hash and the collision-resolution strategy of
// the map interact strongly (a map that mixes the hash itself is insensitive to
// a weak hash; one that does not is very sensitive to it), so to evaluate them
// you want to be able to vary the two separately. The CMake options
// `SLANG_HASH` and `SLANG_HASHMAP` are therefore free to be combined in any
// cross product.
//

// The available implementations. `SLANG_HASH_IMPL` is defined to one of these by
// the build system; it must be the same for every translation unit, since the
// hash of a key participates in the layout of every `Dictionary`.
#define SLANG_HASH_WYHASH 1
#define SLANG_HASH_BOOST 2
#define SLANG_HASH_ABSL 3
#define SLANG_HASH_STD 4

#ifndef SLANG_HASH_IMPL
#define SLANG_HASH_IMPL SLANG_HASH_WYHASH
#endif

// `ankerl::unordered_dense::hash` is included whichever implementation is
// selected, because its `is_avalanching` marker is what we use to decide *which
// types* get the library hash at all (see `isLibraryHashable` below). Only the
// trait is used when another implementation is selected, so this costs nothing
// at runtime.
#include <ankerl/unordered_dense.h>

#if SLANG_HASH_IMPL == SLANG_HASH_BOOST
#include <boost/container_hash/hash.hpp>
#elif SLANG_HASH_IMPL == SLANG_HASH_ABSL
#include <absl/hash/hash.h>
#elif SLANG_HASH_IMPL == SLANG_HASH_STD
#include <functional>
#endif

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace Slang
{
namespace HashImpl
{

/// True for the types we hand to the selected library's hash function rather
/// than to a Slang-defined `getHashCode()`: the built-in integer, enumeration
/// and pointer types, the smart pointer types, and the standard string types.
///
/// The membership test is "does `ankerl::unordered_dense::hash<T>` advertise
/// itself as avalanching", which is exactly the set of types that library
/// specialises rather than forwarding to `std::hash`. We keep using it as the
/// arbiter even when another hash implementation is selected, for two reasons.
/// It is a set that `boost::hash`, `absl::Hash` and `std::hash` all cover too,
/// so the answer does not depend on which implementation is selected -- only the
/// hash values do, which is the variable we want to isolate. And unlike simply
/// asking whether the selected hash is invocable, it does not silently swallow a
/// type that should have defined `getHashCode()`: `ankerl`'s and `absl`'s
/// primary templates are not SFINAE-friendly, so such a type would be reported
/// as hashable and then fail deep inside the library instead of at the
/// "No hash implementation found for this type" static assertion.
template<typename T, typename = void>
constexpr static bool isLibraryHashable = false;
template<typename T>
constexpr static bool
    isLibraryHashable<T, typename ankerl::unordered_dense::hash<T>::is_avalanching> = true;

#if SLANG_HASH_IMPL == SLANG_HASH_WYHASH

template<typename T>
using LibraryHash = ankerl::unordered_dense::hash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "ankerl::unordered_dense::hash (wyhash)";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return ankerl::unordered_dense::detail::wyhash::hash(buffer, len);
}

#elif SLANG_HASH_IMPL == SLANG_HASH_BOOST

template<typename T>
using LibraryHash = boost::hash<T>;

// `boost::hash` is the identity function for the integer types, so its low bits
// carry no more entropy than the key's do.
constexpr bool kIsAvalanching = false;
constexpr const char* kName = "boost::hash";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return boost::hash_range(buffer, buffer + len);
}

#elif SLANG_HASH_IMPL == SLANG_HASH_ABSL

template<typename T>
using LibraryHash = absl::Hash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "absl::Hash";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return absl::Hash<std::string_view>{}(len ? std::string_view(buffer, len) : std::string_view());
}

#elif SLANG_HASH_IMPL == SLANG_HASH_STD

template<typename T>
using LibraryHash = std::hash<T>;

// libstdc++ and MSVC both use the identity function for the integer types.
constexpr bool kIsAvalanching = false;
constexpr const char* kName = "std::hash";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return std::hash<std::string_view>{}(len ? std::string_view(buffer, len) : std::string_view());
}

#else
#error "SLANG_HASH_IMPL is not set to one of the SLANG_HASH_* values"
#endif

} // namespace HashImpl

/// Returns the name of the hash function this build of Slang was compiled
/// against, e.g. "absl::Hash". Use it to confirm which binary you are measuring.
inline const char* getHashImplName()
{
    return HashImpl::kName;
}

} // namespace Slang

#endif
