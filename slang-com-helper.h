#ifndef SLANG_COM_HELPER_H
#define SLANG_COM_HELPER_H

/** \file slang-com-helper.h
*/

#include "slang.h"

/* !!!!!!!!!!!!!!!!!!!!! Macros to help checking SlangResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*! Set SLANG_HANDLE_RESULT_FAIL(x) to code to be executed whenever an error occurs, and is detected by one of the macros */
#ifndef SLANG_HANDLE_RESULT_FAIL
#	define SLANG_HANDLE_RESULT_FAIL(x)
#endif

//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result. 
#define SLANG_RETURN_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return _res; } }
//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define SLANG_RETURN_VOID_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return; } }
//! Helper macro that will return false on failure.
#define SLANG_RETURN_FALSE_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return false; } }
//! Helper macro that will return nullptr on failure.
#define SLANG_RETURN_NULL_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return nullptr; } }

//! Helper macro that will assert if the return code from a call is failure, also returns the failure.
#define SLANG_ASSERT_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { assert(false); return _res; } }
//! Helper macro that will assert if the result from a call is a failure, also returns. 
#define SLANG_ASSERT_VOID_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { assert(false); return; } }

/* !!!!!!!!!!!!!!!!!!!!!!! C++ helpers !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#if defined(__cplusplus)
namespace Slang {

// Alias SlangResult to Slang::Result
typedef SlangResult Result;
// Alias SlangUUID to Slang::Guid
typedef SlangUUID Guid;

SLANG_FORCE_INLINE bool operator==(const Guid& aIn, const Guid& bIn)
{
    // Use the largest type the honors the alignment of Guid
    typedef uint32_t CmpType;
    union GuidCompare
    {
        Guid guid;
        CmpType data[sizeof(Guid) / sizeof(CmpType)];
    };
    // Type pun - so compiler can 'see' the pun and not break aliasing rules
    const CmpType* a = reinterpret_cast<const GuidCompare&>(aIn).data;
    const CmpType* b = reinterpret_cast<const GuidCompare&>(bIn).data;
    // Make the guid comparison a single branch, by not using short circuit
    return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3])) == 0;
}

SLANG_FORCE_INLINE bool operator!=(const Guid& a, const Guid& b)
{
    return !(a == b);
}

} // namespace Slang
#endif // defined(__cplusplus)

#endif
