#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#include "slang-math.h"
#include <string.h>
#include <type_traits>

namespace Slang
{
    typedef int HashCode;

	inline int GetHashCode(double key)
	{
		return FloatAsInt((float)key);
	}
	inline int GetHashCode(float key)
	{
		return FloatAsInt(key);
	}
	inline int GetHashCode(const char * buffer)
	{
		if (!buffer)
			return 0;
		int hash = 0;
		int c;
		auto str = buffer;
		c = *str++;
		while (c)
		{
			hash = c + (hash << 6) + (hash << 16) - hash;
			c = *str++;
		}
		return hash;
	}
	inline int GetHashCode(char * buffer)
	{
		return GetHashCode(const_cast<const char *>(buffer));
	}
    inline int GetHashCode(const char * buffer, size_t numChars)
    {
        int hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {      
            hash = int(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    inline uint64_t GetHashCode64(const char * buffer, size_t numChars)
    {
        // Use uints because hash requires wrap around behavior and int is undefined on over/underflows
        uint64_t hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {
            hash = uint64_t(int64_t(buffer[i])) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

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
		static int GetHashCode(TKey & key)
		{
			return (int)key;
		}
	};
	template<>
	class Hash<0>
	{
	public:
		template<typename TKey>
		static int GetHashCode(TKey & key)
		{
			return int(key.GetHashCode());
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
		static int GetHashCode(TKey const& key)
		{
			return (int)((PtrInt)key) / 16; // sizeof(typename std::remove_pointer<TKey>::type);
		}
	};
	template<>
	class PointerHash<0>
	{
	public:
		template<typename TKey>
		static int GetHashCode(TKey & key)
		{
			return Hash<std::is_integral<TKey>::value || std::is_enum<TKey>::value>::GetHashCode(key);
		}
	};

	template<typename TKey>
	int GetHashCode(const TKey & key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::GetHashCode(key);
	}

	template<typename TKey>
	int GetHashCode(TKey & key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::GetHashCode(key);
	}

    inline int combineHash(int left, int right)
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
            m_hashCode = combineHash(m_hashCode, GetHashCode(value));
        }

        template<typename T>
        void hashObject(T const& object)
        {
            m_hashCode = combineHash(m_hashCode, object->GetHashCode());
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
