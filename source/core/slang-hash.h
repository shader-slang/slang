#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#include "slang-math.h"
#include <string.h>
#include <type_traits>

namespace Slang
{
    // Ideally Hash codes should be unsigned types - makes accumulation simpler (as overflow/underflow behavior are defined)
    // Only downside is around multiply, where unsigned multiply can be slightly slower on some targets.

    // HashCode - size may vary by platform. Typically has 'best' combination of bits/performance. Should not be exposed externally as value from same input may change depending on compilation platform.
    typedef unsigned int HashCode;

    // A fixed 64bit wide hash on all targets.
    typedef uint64_t HashCode64;
    // A fixed 32bit wide hash on all targets.
    typedef uint32_t HashCode32;

    SLANG_FORCE_INLINE HashCode32 toHash32(HashCode value) { return (sizeof(HashCode) == sizeof(int64_t)) ? (HashCode32(uint64_t(value) >> 32) ^ HashCode(value)) : HashCode32(value); }
    SLANG_FORCE_INLINE HashCode64 toHash64(HashCode value) { return (sizeof(HashCode) == sizeof(int64_t)) ? HashCode(value) : ((HashCode64(value) << 32) | value); }

    SLANG_FORCE_INLINE HashCode getHashCode(int64_t value)
    {
        return (sizeof(HashCode) == sizeof(int64_t)) ? HashCode(value) : (HashCode(uint64_t(value) >> 32) ^ HashCode(value));
    }
    SLANG_FORCE_INLINE HashCode getHashCode(uint64_t value)
    {
        return (sizeof(HashCode) == sizeof(uint64_t)) ? HashCode(value) : (HashCode(value >> 32) ^ HashCode(value));
    }

	inline HashCode getHashCode(double key)
	{
		return getHashCode(DoubleAsInt64(key));
	}
	inline HashCode getHashCode(float key)
	{
		return FloatAsInt(key);
	} 
	inline HashCode getHashCode(const char* buffer)
	{
		if (!buffer)
			return 0;
		HashCode hash = 0;
		auto str = buffer;
		HashCode c = HashCode(*str++);
		while (c)
		{
			hash = c + (hash << 6) + (hash << 16) - hash;
			c = HashCode(*str++);
		}
		return hash;
	} 
	inline HashCode getHashCode(char* buffer)
	{
		return getHashCode(const_cast<const char *>(buffer));
	}
    inline HashCode getHashCode(const char* buffer, size_t numChars)
    {
        HashCode hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {      
            hash = HashCode(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
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

    // Hash functions with specific sized results
    // TODO(JS): We might want to implement HashCode as just an alias a suitable Hash32/Hash32 based on target.
    // For now just use Stable for 64bit.
    SLANG_FORCE_INLINE HashCode64 getHashCode64(const char* buffer, size_t numChars) { return getStableHashCode64(buffer, numChars); }
    SLANG_FORCE_INLINE HashCode32 getHashCode32(const char* buffer, size_t numChars) { return toHash32(getHashCode(buffer, numChars)); }

	template<int IsInt>
	class Hash
	{
	public:
	};
	template<>
	class Hash<1>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return (HashCode)key;
		}
	};
	template<>
	class Hash<0>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return HashCode(key.getHashCode());
		}
	};
	template<int IsPointer>
	class PointerHash
	{};
	template<>
	class PointerHash<1>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey const& key)
		{
			return (HashCode)((PtrInt)key) / 16; // sizeof(typename std::remove_pointer<TKey>::type);
		}
	};
	template<>
	class PointerHash<0>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return Hash<std::is_integral<TKey>::value || std::is_enum<TKey>::value>::getHashCode(key);
		}
	};

	template<typename TKey>
	HashCode getHashCode(const TKey& key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::getHashCode(key);
	}

	template<typename TKey>
	HashCode getHashCode(TKey& key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::getHashCode(key);
	}

    inline HashCode combineHash(HashCode left, HashCode right)
    {
        return (left * 16777619) ^ right;
    }

    struct Hasher
    {
    public:
        Hasher() {}

        template<typename T>
        void hashValue(T const& value)
        {
            m_hashCode = combineHash(m_hashCode, getHashCode(value));
        }

        template<typename T>
        void hashObject(T const& object)
        {
            m_hashCode = combineHash(m_hashCode, object->getHashCode());
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
