#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#include "slang-math.h"
#include "slang.h"

#include <ankerl/unordered_dense.h>
#include <cstring>
#include <type_traits>

// For `_umul128`, which `HashDetail::multiply64To128` uses where the compiler
// has no 128-bit integer type of its own.
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif

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

// Does the hashmap implementation provide a uniform hash for this type.
template<typename T, typename = void>
constexpr static bool HasWyhash = false;
template<typename T>
constexpr static bool HasWyhash<T, typename ankerl::unordered_dense::hash<T>::is_avalanching> =
    true;

// We want to have an associated type 'is_avalanching = void' iff we have a
// hash with good uniformity, the two specializations here add that member
// when appropriate (since we can't declare an associated type with
// constexpr if or something terse like that)
template<typename T, typename = void>
struct DetectAvalanchingHash
{
};
template<typename T>
struct DetectAvalanchingHash<T, std::enable_if_t<HasWyhash<T>>>
{
    using is_avalanching = void;
};
// Have we marked 'getHashCode' as having good uniformity properties.
//
// Declaring `static constexpr bool kHasUniformHash = true` is a promise to the
// hash map that this type's `getHashCode` is already well distributed in every
// bit, and the map responds by skipping the mixing step it would otherwise
// apply. Make that promise when the hash is built out of `combineHash`,
// `Hasher`, `hashObjectBytes` or `hashBytes`, all of which mix thoroughly, or
// when it simply forwards to a hash that does.
//
// Do not make it for a hash that merely combines its inputs by hand.
// `UIntSet::getHashCode`, for instance, xors its elements together and returns
// a signed `int`; it is not used as a dictionary key, and it must not be
// marked if it ever becomes one. A wrong promise here is invisible -- nothing
// fails, the map just clusters.
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
        // Otherwise fall back to any good hash provided by the hashmap
        // library
        else if constexpr (HasWyhash<T>)
            return ankerl::unordered_dense::hash<T>{}(t);
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
    return ankerl::unordered_dense::detail::wyhash::hash(buffer, len);
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

#define SLANG_COMPONENTWISE_HASHABLE_1 \
    auto getHashCode() const           \
    {                                  \
        const auto& [m1] = *this;      \
        return Slang::getHashCode(m1); \
    }

#define SLANG_COMPONENTWISE_HASHABLE_2                                          \
    auto getHashCode() const                                                    \
    {                                                                           \
        const auto& [m1, m2] = *this;                                           \
        return combineHash(::Slang::getHashCode(m1), ::Slang::getHashCode(m2)); \
    }

namespace HashDetail
{
/// Multiplies `a` by `b` as a 128-bit product, leaving the low half in `a` and
/// the high half in `b`.
///
/// This is wyhash's `mum`. It is spelled out here, rather than called through
/// a hash map library that happens to vendor a copy of wyhash, because which
/// hash map Slang is built against is a build-time choice and `combineHash`
/// must not change or stop compiling when that choice does.
inline void multiply64To128(HashCode64& a, HashCode64& b)
{
#if defined(__SIZEOF_INT128__)
    const __uint128_t product = __uint128_t(a) * __uint128_t(b);
    a = HashCode64(product);
    b = HashCode64(product >> 64);
#elif defined(_MSC_VER) && defined(_M_X64)
    a = _umul128(a, b, &b);
#else
    // Four 32x32 -> 64 bit products, recombined by hand.
    const HashCode64 highA = a >> 32, lowA = HashCode64(uint32_t(a));
    const HashCode64 highB = b >> 32, lowB = HashCode64(uint32_t(b));
    const HashCode64 highProduct = highA * highB;
    const HashCode64 crossA = highA * lowB;
    const HashCode64 crossB = highB * lowA;
    const HashCode64 lowProduct = lowA * lowB;

    const HashCode64 partial = lowProduct + (crossA << 32);
    const HashCode64 carryFromPartial = HashCode64(partial < lowProduct);
    const HashCode64 low = partial + (crossB << 32);
    const HashCode64 carryFromLow = HashCode64(low < partial);

    a = low;
    b = highProduct + (crossA >> 32) + (crossB >> 32) + carryFromPartial + carryFromLow;
#endif
}

/// wyhash's MUM: the 128-bit product of `a` and `b`, with its halves xored
/// together. Multiplying is what carries entropy upwards and the xor is what
/// brings the top half back down, so every input bit reaches the whole result.
inline HashCode64 mumMix(HashCode64 a, HashCode64 b)
{
    multiply64To128(a, b);
    return a ^ b;
}

/// Folds `m` into the running hash `h`, and returns a value that is
/// well-distributed in every bit.
///
/// The step this replaces was `h * 16777619 ^ m`, which only ever moves
/// entropy *upwards*: multiplying shifts bits left, and the trailing xor lets
/// `m` contribute to a bit only if `m` itself has entropy there. Almost
/// everything Slang folds this way is a pointer, a small enumeration or a
/// bool, so the low bits of `m` are fixed, and the low bits of the result
/// learned nothing from it.
///
/// That was measurable rather than theoretical. Emulating `ValNodeDesc::init`
/// over twenty thousand randomly generated four-operand descriptors sharing
/// one node tag, the bottom three bits of the hash took exactly one value: for
/// any given (node tag, operand count) they were a constant, whatever the
/// operands were. A hash map that takes its bucket index from the low bits and
/// does not re-mix the hash itself -- which is a choice each implementation
/// makes differently -- then confines an entire class of keys to one eighth of
/// the table.
///
/// The actual mixing is MUM -- multiply two 64-bit values into a 128-bit
/// product and xor its halves back together. That is wyhash's core primitive,
/// reproduced here rather than reached for in whichever hash map library
/// happens to be linked, so that this does not break when a different one is
/// selected. It is used instead of a hand-rolled multiply-and-shift because
/// hand-rolling one is easy to get subtly wrong: an earlier attempt left a
/// flip of the top bit of an operand changing only two bits of the result, and
/// never reaching the bucket index at all.
///
/// How the accumulator is brought in is Slang's own choice. wyhash-derived
/// combiners generally fold with `mix(h + m, k)`, which is symmetric in `h`
/// and `m`, so a two-element key would hash the same as its own reversal --
/// and several keys here are a pair of same-typed members (`TypePair` holds
/// two `Type*`, `ConversionMethodKey` a from- and a to-type). Rotating the
/// accumulator before folding `m` in makes the step order-dependent, for the
/// cost of one rotate.
///
/// The trailing shift-xor is there because one MUM alone is not quite enough
/// for a *fold*. Measured by flipping each bit of an operand in turn and asking
/// how often the bottom six bits of the result change, a bare
/// `mumMix(rotated ^ m, k)` left one bit reaching the bucket index only 17% of
/// the time, against 35% for the worst bit of wyhash's own integer hash, which
/// is what every pointer-keyed dictionary here already relies on. Folding the
/// high half down afterwards takes the worst bit to 89%, for one shift and one
/// xor. A second MUM would reach 98% and is not worth it on a step that runs
/// once per operand.
inline HashCode64 foldHashStep(HashCode64 h, HashCode64 m)
{
    const HashCode64 rotated = (h << 23) | (h >> 41);
    const HashCode64 mixed = mumMix(rotated ^ m, 0x9DDFEA08EB382D69ULL);
    return mixed ^ (mixed >> 29);
}
/// The 32-bit form of `foldHashStep`, for chains that are entirely 32-bit: the
/// same shape, with a 32x32 -> 64 bit product standing in for MUM.
inline HashCode32 foldHashStep(HashCode32 h, HashCode32 m)
{
    const HashCode32 rotated = (h << 13) | (h >> 19);
    const uint64_t product = uint64_t(rotated ^ m) * 0x9DDFEA08U;
    const HashCode32 mixed = HashCode32(product) ^ HashCode32(product >> 32);
    return mixed ^ (mixed >> 15);
}

/// The width `combineHash` folds in: the wider of its two operands, so that
/// mixing a 32-bit value into a 64-bit hash does not discard the top half.
template<typename H1, typename H2>
using FoldType = std::conditional_t<(sizeof(H1) > 4 || sizeof(H2) > 4), HashCode64, HashCode32>;
} // namespace HashDetail

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
    using Fold = HashDetail::FoldType<H1, H2>;
    return combineHash(
        HashDetail::foldHashStep(
            static_cast<Fold>(static_cast<std::make_unsigned_t<H1>>(n)),
            static_cast<Fold>(static_cast<std::make_unsigned_t<H2>>(m))),
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
