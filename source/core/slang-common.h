#ifndef SLANG_CORE_COMMON_H
#define SLANG_CORE_COMMON_H

#include "../../slang.h"

#include <assert.h>

#include <stdint.h>

#ifdef __GNUC__
#define CORE_LIB_ALIGN_16(x) x __attribute__((aligned(16)))
#else
#define CORE_LIB_ALIGN_16(x) __declspec(align(16)) x
#endif

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

//	typedef unsigned short Word;

	typedef intptr_t PtrInt;

    // Type used for indexing, in arrays/views etc
    typedef Int Index;

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

#ifdef _MSC_VER
#   define SLANG_RETURN_NEVER __declspec(noreturn)
//#elif SLANG_CLANG
//#   define SLANG_RETURN_NEVER [[noreturn]]
#else
#   define SLANG_RETURN_NEVER [[noreturn]]
//#   define SLANG_RETURN_NEVER /* empty */
#endif

#ifdef _MSC_VER
#define UNREACHABLE_RETURN(x)
#define UNREACHABLE(x)
#else
#define UNREACHABLE_RETURN(x) return x;
#define UNREACHABLE(x) x;
#endif

    SLANG_RETURN_NEVER void signalUnexpectedError(char const* message);
}

#define SLANG_UNEXPECTED(reason) \
    Slang::signalUnexpectedError("unexpected: " reason)

#define SLANG_UNIMPLEMENTED_X(what) \
    Slang::signalUnexpectedError("unimplemented: " what)

#define SLANG_UNREACHABLE(msg) \
    Slang::signalUnexpectedError("unreachable code executed: " msg)

#ifdef _DEBUG
#define SLANG_EXPECT(VALUE, MSG) if(VALUE) {} else Slang::signalUnexpectedError("assertion failed: '" MSG "'")
#define SLANG_ASSERT(VALUE) SLANG_EXPECT(VALUE, #VALUE)
#else
#define SLANG_EXPECT(VALUE, MSG) do {} while(0)
#define SLANG_ASSERT(VALUE) do {} while(0)
#endif

#define SLANG_RELEASE_ASSERT(VALUE) if(VALUE) {} else Slang::signalUnexpectedError("assertion failed")
#define SLANG_RELEASE_EXPECT(VALUE, WHAT) if(VALUE) {} else SLANG_UNEXPECTED(WHAT)

template<typename T> void slang_use_obj(T&) {}

#define SLANG_UNREFERENCED_PARAMETER(P) slang_use_obj(P)
#define SLANG_UNREFERENCED_VARIABLE(P) slang_use_obj(P)
#endif
