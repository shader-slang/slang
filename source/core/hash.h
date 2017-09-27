#ifndef CORELIB_HASH_H
#define CORELIB_HASH_H

#include "slang-math.h"
#include <string.h>
#include <type_traits>

namespace Slang
{
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
			return key.GetHashCode();
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
}

#endif