#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#include "slang-hash-impl.h"
#include "slang-math.h"
#include "slang.h"

#include <cstring>
#include <type_traits>

namespace Slang
{
//
// Types
//

// A fixed 64bit wide hash on all targets.
typedef uint64_t HashCode64;
typedef HashCode64 HashCode;
// A fixed 32bit wide hash on all targets.
typedef uint32_t HashCode32;

//
// Some helpers to determine which hash to use for a type
//

// Forward declare Hash
template<typename T>
struct Hash;

template<typename T, typename = void>
constexpr static bool HasSlangHash = false;
template<typename T>
constexpr static bool HasSlangHash<
    T,
    std::enable_if_t<
        std::is_convertible_v<decltype((std::declval<const T&>()).getHashCode()), HashCode64>>> =
    true;

// Does the selected hash implementation (see slang-hash-impl.h) provide a hash
// for this type, so that we don't need a Slang-defined one.
template<typename T>
constexpr static bool HasLibraryHash = HashImpl::isLibraryHashable<T>;

// We want to have an associated type 'is_avalanching = void' iff we have a
// hash with good uniformity, the two specializations here add that member
// when appropriate (since we can't declare an associated type with
// constexpr if or something terse like that)
template<typename T, typename = void>
struct DetectAvalanchingHash
{
};
template<typename T>
struct DetectAvalanchingHash<
    T,
    std::enable_if_t<HasLibraryHash<T> && HashImpl::kIsAvalanching>>
{
    using is_avalanching = void;
};
// Have we marked 'getHashCode' as having good uniformity properties.
template<typename T>
struct DetectAvalanchingHash<T, std::enable_if_t<T::kHasUniformHash>>
{
    using is_avalanching = void;
};

// A helper for hashing according to the bit representation
template<typename T, typename U>
struct BitCastHash : DetectAvalanchingHash<U>
{
    auto operator()(const T& t) const
    {
        // Doesn't discard or invent bits
        static_assert(sizeof(T) == sizeof(U));
        // Can we copy bytes to and fro
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(std::is_trivially_copyable_v<U>);
        // Because we construct a U to memcpy into
        static_assert(std::is_trivially_constructible_v<U>);

        U u;
        memcpy(&u, &t, sizeof(T));
        return Hash<U>{}(u);
    }
};

//
// Our hashing functor which disptaches to the most appropriate hashing
// function for the type
//

template<typename T>
struct Hash : DetectAvalanchingHash<T>
{
    auto operator()(const T& t) const
    {
        // Our preference is for any hash we've defined ourselves
        if constexpr (HasSlangHash<T>)
            return t.getHashCode();
        // Otherwise fall back to the hash provided by the selected hash
        // library
        else if constexpr (HasLibraryHash<T>)
            return HashImpl::LibraryHash<T>{}(t);
        // Otherwise fail
        else
        {
            // !sizeof(T*) is a 'false' which is dependent on T (pending P2593R0)
            static_assert(!sizeof(T*), "No hash implementation found for this type");
            // This is to avoid the return type being deduced as 'void' and creating further errors.
            return HashCode64(0);
        }
    }
};

// Specializations for float and double which hash 0 and -0 to distinct values
template<>
struct Hash<float> : BitCastHash<float, uint32_t>
{
};
template<>
struct Hash<double> : BitCastHash<double, uint64_t>
{
};

//
// Utility functions for using hashes
//

// A wrapper for Hash<TKey>
template<typename TKey>
auto getHashCode(const TKey& key)
{
    return Hash<TKey>{}(key);
}

inline HashCode64 getHashCode(const char* buffer, std::size_t len)
{
    return HashImpl::hashBytes(buffer, len);
}

template<typename T>
HashCode64 hashObjectBytes(const T& t)
{
    static_assert(
        std::has_unique_object_representations_v<T>,
        "This type must have a unique object representation to use hashObjectBytes");
    return getHashCode(reinterpret_cast<const char*>(&t), sizeof(t));
}

// Use in a struct to declare a uniform hash which doens't care about the
// structure of the members.
#define SLANG_BYTEWISE_HASHABLE                   \
    static constexpr bool kHasUniformHash = true; \
    ::Slang::HashCode64 getHashCode() const       \
    {                                             \
        return ::Slang::hashObjectBytes(*this);   \
    }

// These spell the return type out as HashCode64 rather than deducing it with
// `auto`. A deduced return type is only known once the function body has been
// parsed, and the body of a member function of a nested class is not parsed
// until the *enclosing* class is complete. So with `auto`, HasSlangHash<T> is
// false for any such nested type while the enclosing class is still being
// defined, and Hash<T> silently falls through to "No hash implementation found
// for this type". Nothing normally asks the question that early, but
// std::unordered_map does: it instantiates __is_fast_hash<Hash> and
// __is_nothrow_invocable<Hash> as part of instantiating the map class itself,
// which happens at the point where the map is declared as a member. See for
// example SPIRVCoreGrammarInfo, which declares Dictionary members keyed by its
// own nested QualifiedEnumName.
#define SLANG_COMPONENTWISE_HASHABLE_1        \
    ::Slang::HashCode64 getHashCode() const   \
    {                                         \
        const auto& [m1] = *this;             \
        return ::Slang::getHashCode(m1);      \
    }

#define SLANG_COMPONENTWISE_HASHABLE_2                                          \
    ::Slang::HashCode64 getHashCode() const                                     \
    {                                                                           \
        const auto& [m1, m2] = *this;                                           \
        return combineHash(::Slang::getHashCode(m1), ::Slang::getHashCode(m2)); \
    }

inline HashCode64 combineHash(HashCode64 h)
{
    return h;
}

inline HashCode32 combineHash(HashCode32 h)
{
    return h;
}

// A left fold of a mixing operation
template<typename H1, typename H2, typename... Hs>
auto combineHash(H1 n, H2 m, Hs... args)
{
    static_assert(
        (std::is_integral_v<std::remove_cv_t<H1>> && !std::is_same_v<std::remove_cv_t<H1>, bool>) ||
        std::is_enum_v<std::remove_cv_t<H1>>);
    static_assert(
        (std::is_integral_v<std::remove_cv_t<H2>> && !std::is_same_v<std::remove_cv_t<H2>, bool>) ||
        std::is_enum_v<std::remove_cv_t<H2>>);
    return combineHash(
        (static_cast<std::make_unsigned_t<H1>>(n) * 16777619U) ^
            static_cast<std::make_unsigned_t<H2>>(m),
        args...);
}

template<typename I>
HashCode64 symmetricHash(I begin, I end)
{
    using V = typename std::iterator_traits<I>::value_type;

    HashCode64 sum = 0;
    HashCode64 product = 1;
    const HashCode64 prime = 0x9e3779b9;

    for (I it = begin; it != end; ++it)
    {
        HashCode64 element_hash = getHashCode(*it);

        sum += element_hash;
        product *= element_hash | 1; // avoid zeros
    }

    product ^= product >> 16;

    return sum ^ (product * prime);
}
struct Hasher
{
public:
    Hasher() {}

    /// Hash the given `value` and combine it into this hash state
    template<typename T>
    void hashValue(T const& value)
    {
        // TODO: Eventually, we should replace `getHashCode`
        // with a "hash into" operation that takes the value
        // and a `Hasher`.

        m_hashCode = combineHash(m_hashCode, getHashCode(value));
    }

    /// Combine the given `hash` code into the hash state.
    ///
    /// Note: users should prefer to use `hashValue` or `hashObject`
    /// when possible, as they may be able to ensure a higher-quality
    /// hash result (e.g., by using more bits to represent the state
    /// during hashing than are used for the final hash code).
    ///
    void addHash(HashCode hash) { m_hashCode = combineHash(m_hashCode, hash); }

    HashCode getResult() const { return m_hashCode; }

private:
    HashCode m_hashCode = 0;
};
} // namespace Slang

#endif
