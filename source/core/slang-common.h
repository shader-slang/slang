#ifndef SLANG_CORE_COMMON_H
#define SLANG_CORE_COMMON_H

#include "../../slang.h"

#include <assert.h>

#include <stdint.h>

#include "slang-signal.h"

#define VARIADIC_TEMPLATE

namespace Slang
{
    
	typedef int32_t Int32;
	typedef uint32_t UInt32;

	typedef int64_t Int64;
	typedef uint64_t UInt64;

    // Define 
    typedef SlangUInt UInt;
    typedef SlangInt Int;

    static const UInt kMaxUInt = ~UInt(0);
    static const Int kMaxInt = Int(kMaxUInt >> 1);

//	typedef unsigned short Word;

	typedef intptr_t PtrInt;

    // TODO(JS): It looks like Index is actually 64 bit on 64 bit targets(!)
    // Previous discussions landed on Index being int32_t.

    // Type used for indexing, in arrays/views etc. Signed.
    typedef Int Index;
    typedef UInt UIndex;
    typedef Int Count;
    typedef UInt UCount;

    static const Index kMaxIndex = kMaxInt;

    typedef uint8_t Byte;

    // TODO(JS):
    // Perhaps these should be named Utf8, Utf16 and UnicodePoint/Rune/etc? For now, just keep it simple
    //
    typedef char Char8;
    // 16 bit character. Note much like in utf8, a character may or may not represent a code point (it can be part of a code point).  
    typedef uint16_t Char16;

    // Can always hold a unicode code point.
    typedef uint32_t Char32;

	template <typename T>
	inline T&& _Move(T & obj)
	{
		return static_cast<T&&>(obj);
	}

	template <typename T>
	inline void Swap(T & v0, T & v1)
	{
		T tmp = _Move(v0);
		v0 = _Move(v1);
		v1 = _Move(tmp);
	}

    // Make these interfaces have more convenient names
    typedef ISlangCastable ICastable;
    typedef ISlangClonable IClonable;

    // Convenience function for using clonable
    template <typename T>
    SLANG_FORCE_INLINE T* clone(IClonable* clonable) { return (T*)clonable->clone(T::getTypeGuid()); }


// TODO: Shouldn't these be SLANG_ prefixed?
#ifdef _MSC_VER
#define UNREACHABLE_RETURN(x)
#define UNREACHABLE(x)
#else
#define UNREACHABLE_RETURN(x) return x;
#define UNREACHABLE(x) x;
#endif

}

#ifdef _DEBUG
#define SLANG_EXPECT(VALUE, MSG) if(VALUE) {} else SLANG_ASSERT_FAILURE(MSG)
#define SLANG_ASSERT(VALUE) SLANG_EXPECT(VALUE, #VALUE)
#else
#define SLANG_EXPECT(VALUE, MSG) do {} while(0)
#define SLANG_ASSERT(VALUE) do {} while(0)
#endif

#define SLANG_RELEASE_ASSERT(VALUE) if(VALUE) {} else SLANG_ASSERT_FAILURE(#VALUE)
#define SLANG_RELEASE_EXPECT(VALUE, WHAT) if(VALUE) {} else SLANG_UNEXPECTED(WHAT)

template<typename T> void slang_use_obj(T&) {}

#define SLANG_UNREFERENCED_PARAMETER(P) slang_use_obj(P)
#define SLANG_UNREFERENCED_VARIABLE(P) slang_use_obj(P)
#endif

#if defined(SLANG_RT_DYNAMIC)
#if defined(_MSC_VER)
#    ifdef SLANG_RT_DYNAMIC_EXPORT
#        define SLANG_RT_API SLANG_DLL_EXPORT
#    else
#        define SLANG_RT_API __declspec(dllimport)
#    endif
#else
// TODO: need to consider compiler capabilities
//#     ifdef SLANG_RT_DYNAMIC_EXPORT
#    define SLANG_RT_API SLANG_DLL_EXPORT
//#     endif
#endif
#endif

#ifndef SLANG_RT_API
#define SLANG_RT_API
#endif
