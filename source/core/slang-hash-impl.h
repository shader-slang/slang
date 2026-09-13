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
#define SLANG_HASH_RAPIDHASH 2
#define SLANG_HASH_KOMIHASH 3
#define SLANG_HASH_XXH3 4
#define SLANG_HASH_BOOST 5
#define SLANG_HASH_ABSL 6
#define SLANG_HASH_STD 7

#ifndef SLANG_HASH_IMPL
#define SLANG_HASH_IMPL SLANG_HASH_WYHASH
#endif

// `ankerl::unordered_dense::hash` is included whichever implementation is
// selected, because its `is_avalanching` marker is what we use to decide *which
// types* get the library hash at all (see `isLibraryHashable` below). Only the
// trait is used when another implementation is selected, so this costs nothing
// at runtime.
#include <ankerl/unordered_dense.h>

#if SLANG_HASH_IMPL == SLANG_HASH_RAPIDHASH
#include <rapidhash.h>
#elif SLANG_HASH_IMPL == SLANG_HASH_KOMIHASH
// Not <komihash.h>: komihash ships as a header-only library whose functions are
// `static`, which is normal and correct for it, but naming one from `hashBytes`
// below -- an inline function with external linkage -- would be ill-formed.
// This declares one ordinary function compiled from it instead. See the
// KOMIHASH branch in external/CMakeLists.txt.
#include <slang-komihash.h>
#elif SLANG_HASH_IMPL == SLANG_HASH_XXH3
// Not <xxhash.h>: lz4 exports a directory holding an older copy of that name,
// which wins on this include path. See the XXH3 branch in external/CMakeLists.txt.
#include <slang-xxhash.h>
#elif SLANG_HASH_IMPL == SLANG_HASH_BOOST
#include <boost/container_hash/hash.hpp>
#elif SLANG_HASH_IMPL == SLANG_HASH_ABSL
#include <absl/hash/hash.h>
#elif SLANG_HASH_IMPL == SLANG_HASH_STD
#include <functional>
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

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

/// Returns a hash of the `len` bytes at `buffer`.
///
/// Declared here and defined once per implementation below, so that
/// `Detail::ByteRangeHash` can name it before the implementation branches are
/// reached.
inline uint64_t hashBytes(const char* buffer, std::size_t len);

namespace Detail
{

/// Hashes a 64-bit word by handing its object representation to `hashBytes`.
inline uint64_t hashWord(uint64_t word)
{
    return hashBytes(reinterpret_cast<const char*>(&word), sizeof(word));
}

template<typename T>
struct IsStdString : std::false_type
{
};
template<typename CharT, typename Traits, typename Allocator>
struct IsStdString<std::basic_string<CharT, Traits, Allocator>> : std::true_type
{
};
template<typename CharT, typename Traits>
struct IsStdString<std::basic_string_view<CharT, Traits>> : std::true_type
{
};

template<typename T>
struct IsStdSmartPointer : std::false_type
{
};
template<typename T, typename Deleter>
struct IsStdSmartPointer<std::unique_ptr<T, Deleter>> : std::true_type
{
};
template<typename T>
struct IsStdSmartPointer<std::shared_ptr<T>> : std::true_type
{
};

template<typename T, typename = void>
constexpr bool isTupleLike = false;
template<typename T>
constexpr bool isTupleLike<T, std::void_t<decltype(std::tuple_size<T>::value)>> = true;

/// The `LibraryHash` for the implementations that hash a byte range and nothing
/// else.
///
/// rapidhash, komihash and XXH3 are each a single C function taking a pointer
/// and a length, where wyhash, `boost::hash`, `absl::Hash` and `std::hash` come
/// with a family of per-type functors. This supplies the per-type layer they
/// lack, and has to cover exactly the set `isLibraryHashable` admits, since that
/// set is deliberately the same whichever implementation is selected: the
/// standard strings hash their characters, raw and smart pointers hash the
/// address, integers and enumerations hash their value widened to 64 bits, and
/// `std::pair`/`std::tuple` hash the array of their elements' hashes.
///
/// Note that this puts the byte-range implementations at a slight disadvantage
/// on the integer and pointer keys that make up most of Slang's.
/// `SLANG_HASH=WYHASH` reaches those through `ankerl::unordered_dense::hash`,
/// whose specializations for them call `wyhash::mix` -- two multiplies --
/// rather than the full `wyhash::hash` over eight bytes that this does. All
/// three of these functions special-case inputs of sixteen bytes or fewer, so
/// the gap is small, but it is real, and it is a reason to read a close result
/// as a tie rather than a win for wyhash.
template<typename T>
struct ByteRangeHash
{
    using is_avalanching = void;

    uint64_t operator()(const T& value) const
    {
        if constexpr (IsStdString<T>::value)
        {
            return hashBytes(
                reinterpret_cast<const char*>(value.data()),
                value.size() * sizeof(typename T::value_type));
        }
        else if constexpr (IsStdSmartPointer<T>::value)
        {
            return hashWord(reinterpret_cast<std::uintptr_t>(value.get()));
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            return hashWord(reinterpret_cast<std::uintptr_t>(value));
        }
        else if constexpr (std::is_enum_v<T>)
        {
            return hashWord(static_cast<uint64_t>(static_cast<std::underlying_type_t<T>>(value)));
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return hashWord(static_cast<uint64_t>(value));
        }
        else if constexpr (isTupleLike<T>)
        {
            return hashElements(value, std::make_index_sequence<std::tuple_size_v<T>>{});
        }
        else
        {
            // !sizeof(T*) is a 'false' which is dependent on T (pending P2593R0)
            static_assert(
                !sizeof(T*),
                "isLibraryHashable admits a type that ByteRangeHash does not handle; teach it "
                "about the type in slang-hash-impl.h");
            return uint64_t(0);
        }
    }

private:
    /// Hashes each element of a pair or tuple to 64 bits, then hashes the array
    /// of those words.
    template<std::size_t... kIndices>
    static uint64_t hashElements(const T& value, std::index_sequence<kIndices...>)
    {
        const uint64_t words[] = {
            ByteRangeHash<std::tuple_element_t<kIndices, T>>{}(std::get<kIndices>(value))...};
        return hashBytes(reinterpret_cast<const char*>(words), sizeof(words));
    }
};

} // namespace Detail

#if SLANG_HASH_IMPL == SLANG_HASH_WYHASH

template<typename T>
using LibraryHash = ankerl::unordered_dense::hash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "ankerl::unordered_dense::hash (wyhash)";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return ankerl::unordered_dense::detail::wyhash::hash(buffer, len);
}

#elif SLANG_HASH_IMPL == SLANG_HASH_RAPIDHASH

template<typename T>
using LibraryHash = Detail::ByteRangeHash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "rapidhash";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    // rapidhash also ships `rapidhashMicro` and `rapidhashNano`, cut-down
    // variants for inputs below 256 and 48 bytes respectively, which covers
    // almost every key Slang hashes. If plain rapidhash measures well it is
    // worth trying those too.
    return rapidhash(buffer, len);
}

#elif SLANG_HASH_IMPL == SLANG_HASH_KOMIHASH

template<typename T>
using LibraryHash = Detail::ByteRangeHash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "komihash";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    // The seed komihash takes is fixed at 0 inside slangKomihash; see the
    // KOMIHASH branch in external/CMakeLists.txt for why the call goes through
    // that rather than straight to komihash().
    return slangKomihash(buffer, len);
}

#elif SLANG_HASH_IMPL == SLANG_HASH_XXH3

template<typename T>
using LibraryHash = Detail::ByteRangeHash<T>;

constexpr bool kIsAvalanching = true;
constexpr const char* kName = "xxHash XXH3";

inline uint64_t hashBytes(const char* buffer, std::size_t len)
{
    return XXH3_64bits(buffer, len);
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
