#ifndef SLANG_RESULT_H
#define SLANG_RESULT_H

#include <cstdint>
#include <assert.h>

#include "../../slang.h"

/*! Set SLANG_HANDLE_RESULT_FAIL(x) to code to be executed whenever an error occurs, and is detected by one of the macros */
#ifndef SLANG_HANDLE_RESULT_FAIL
#	define SLANG_HANDLE_RESULT_FAIL(x)
#endif

/* !!!!!!!!!!!!!!!!!!!!!!!!! Checking codes !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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

#if defined(__cplusplus)
namespace Slang {
typedef SlangResult Result;
} // namespace Slang
#endif // defined(__cplusplus)

#endif // SLANG_RESULT_H