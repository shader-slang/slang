#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#include "../../slang.h"
#include "slang-math.h"
#include "ankerl/unordered_dense.h"
#include <bit>
#include <concepts>
#include <cstring>
#include <type_traits>

namespace Slang
{
    //
    // Types
    //

    // Ideally Hash codes should be unsigned types - makes accumulation simpler (as overflow/underflow behavior are defined)
    // Only downside is around multiply, where unsigned multiply can be slightly slower on some targets.

    // HashCode - size may vary by platform. Typically has 'best' combination of bits/performance. Should not be exposed externally as value from same input may change depending on compilation platform.
    typedef unsigned int HashCode;

    // A fixed 64bit wide hash on all targets.
    typedef uint64_t HashCode64;
    // A fixed 32bit wide hash on all targets.
    typedef uint32_t HashCode32;

    //
    // Some helpers to determine which hash to use for a type
    //

    // Forward declare Hash
    template<typename T> struct Hash;

    // "Do we have a suitable member function 'getHashCode'?"
    template <typename T>
    concept HasSlangHash = requires(const T &t)
    {
        { t.getHashCode() } -> std::convertible_to<HashCode64>;
    };

    // Have we marked 'getHashCode' as having good uniformity properties.
    template <typename T>
    concept HasUniformHash = T::kHasUniformHash;

    // Does the hashmap implementation provide a uniform hash for this type.
    template <typename T>
    concept HasWyhash = requires { typename ankerl::unordered_dense::hash<T>::is_avalanching; };

    // We want to have an associated type 'is_avalanching = void' iff we have a
    // hash with good uniformity, the two specializations here add that member
    // when appropriate (since we can't declare an associated type with
    // constexpr if or something terse like that)
    template<typename T>
    struct DetectAvalanchingHash {};
    template<HasWyhash T>
    struct DetectAvalanchingHash<T> { using is_avalanching = void; };
    template<HasUniformHash T>
    struct DetectAvalanchingHash<T> { using is_avalanching = void; };

    // A helper for hashing according to the bit representation
    template<typename T, typename U>
    struct BitCastHash : DetectAvalanchingHash<U>
    {
        auto operator()(const T& t) const
        {
            return Hash<U>{}(std::bit_cast<U>(t));
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
    struct Hash<float> : BitCastHash<float, uint32_t> {};
    template<>
    struct Hash<double> : BitCastHash<double, uint64_t> {};

    //
    // Utility functions for using hashes
    //

    // A wrapper for Hash<TKey>
	template<typename TKey>
	auto getHashCode(const TKey& key)
	{
        return Hash<TKey>{}(key);
	}

	inline auto getHashCode(const char* buffer, std::size_t len)
	{
        return ankerl::unordered_dense::detail::wyhash::hash(buffer, len);
	}

    template<typename T>
    HashCode64 hashObjectBytes(const T& t)
    {
        static_assert(std::has_unique_object_representations_v<T>,
            "This type must have a unique object representation to use hashObjectBytes");
        return getHashCode(reinterpret_cast<const char*>(&t), sizeof(t));
    }

    // Use in a struct to declare a uniform hash which doens't care about the
    // structure of the members.
#   define SLANG_BYTEWISE_HASHABLE \
        static constexpr bool kHasUniformHash = true; \
        ::Slang::HashCode64 getHashCode() const \
        { \
            return ::Slang::hashObjectBytes(*this); \
        }


    /* The 'Stable' hash code functions produce hashes that must be

    * The same result for the same inputs on all targets
    * Rarely change - as their values can change the output of the Slang API/Serialization

    Hash value used from the 'Stable' functions can also be used as part of serialization -
    so it is in effect part of the API.

    In effect this means changing a 'Stable' algorithm will typically require doing a new release.
    */
    inline HashCode32 getStableHashCode32(const char* buffer, size_t numChars)
    {
        HashCode32 hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {
            hash = HashCode32(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    inline HashCode64 getStableHashCode64(const char* buffer, size_t numChars)
    {
        // Use HashCode64 is assumed unsigned because hash requires wrap around behavior and int is undefined on over/underflows
        HashCode64 hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {
            hash = HashCode64(HashCode64(buffer[i])) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    inline HashCode combineHash(HashCode h)
    {
        return h;
    }

    template<typename... Hs>
    HashCode combineHash(HashCode n, Hs... args)
    {
        return (n * 16777619) ^ combineHash(args...);
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
        void addHash(HashCode hash)
        {
            m_hashCode = combineHash(m_hashCode, hash);
        }

        HashCode getResult() const
        {
            return m_hashCode;
        }

    private:
        HashCode m_hashCode = 0;
    };
}

#endif
