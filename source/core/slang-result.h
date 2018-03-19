#ifndef SLANG_RESULT_H
#define SLANG_RESULT_H

#include <cstdint>
#include <assert.h>

/*! SlangResult is a type that represents the status result after a call to a function/method. It can represent if the call was successful or 
not. It can also specify in an extensible manner what facility produced the result (as the integral 'facility') as well as what caused it (as an integral 'code').
Under the covers SlangResult is represented as a int32_t. A negative value indicates an error, a positive (or 0) value indicates success. 

SlangResult is designed to be compatible with COM HRESULT. 

It's layout in bits is as follows

Severity | Facility | Code
---------|----------|-----
31       |    30-16 | 15-0

Severity - 1 fail, 0 is success.
Facility is where the error originated from
Code is the code specific to the facility.

The layout is designed such that failure is a negative number, and success is positive due to Result
being represented by an Int32.

Result codes have the following style

1. SLANG_name
2. SLANG_s_f_name
3. SLANG_s_name

where s is severity as a single letter S - success, and E for error
Style 1 is reserved for SLANG_OK and SLANG_FAIL as they are so common and not tied to a facility

s is S for success, E for error 
f is the short version of the facility name

For the common used SLANG_OK and SLANG_FAIL, the name prefix is dropped.
It is acceptable to expand 'f' to a longer name to differentiate a name
ie for a facility 'DRIVER' it might make sense to have an error of the form SLANG_E_DRIVER_OUT_OF_MEMORY 
*/

typedef int32_t SlangResult;

// Make just the identifier for the code
#define SLANG_MAKE_RESULT_ID(fac, code) ((((int32_t)(fac))<<16) | ((int32_t)(code)))

//! Make a result
#define SLANG_MAKE_RESULT(sev, fac, code) ((((int32_t)(sev))<<31) | SLANG_MAKE_RESULT_ID(fac, code))

//! Will be 0 - for ok, 1 for failure
#define SLANG_GET_RESULT_SEVERITY(r)	((int32_t)(((uint32_t(r)) >> 31))
//! Get the facility the result is associated with
#define SLANG_GET_RESULT_FACILITY(r)	((int32_t)(((r) >> 16) & 0x7fff))
//! Get the result code for the facility 
#define SLANG_GET_RESULT_CODE(r)		((int32_t)((r) & 0xffff))

#define SLANG_MAKE_ERROR(fac, code)		(SLANG_MAKE_RESULT_ID(SLANG_FACILITY_##fac, code) | 0x80000000)
#define SLANG_MAKE_SUCCESS(fac, code)	SLANG_MAKE_RESULT_ID(SLANG_FACILITY_##fac, code)

/*************************** Facilities ************************************/

//! General - careful to make compatible with HRESULT
#define SLANG_FACILITY_GENERAL      0

//! Base facility -> so as to not clash with HRESULT values
#define SLANG_FACILITY_BASE		 0x100				

/*! Facilities numbers must be unique across a project to make the resulting result a unique number!
It can be useful to have a consistent short name for a facility, as used in the name prefix */
#define SLANG_FACILITY_DISK         (SLANG_FACILITY_BASE + 1)
#define SLANG_FACILITY_INTERFACE    (SLANG_FACILITY_BASE + 2)
#define SLANG_FACILITY_UNKNOWN      (SLANG_FACILITY_BASE + 3)
#define SLANG_FACILITY_MEMORY		(SLANG_FACILITY_BASE + 4)
#define SLANG_FACILITY_MISC			(SLANG_FACILITY_BASE + 5)

/// Base for external facilities. Facilities should be unique across modules. 
#define SLANG_FACILITY_EXTERNAL_BASE 0x210
#define SLANG_FACILITY_CORE			(SLANG_FACILITY_EXTERNAL_BASE + 1)

/* *************************** Codes **************************************/

// Memory
#define SLANG_E_MEM_OUT_OF_MEMORY            SLANG_MAKE_ERROR(MEMORY, 1)
#define SLANG_E_MEM_BUFFER_TOO_SMALL         SLANG_MAKE_ERROR(MEMORY, 2)

//! SLANG_OK indicates success, and is equivalent to SLANG_MAKE_RESULT(0, GENERAL, 0)
#define SLANG_OK                          0
//! SLANG_FAIL is the generic failure code - meaning a serious error occurred and the call couldn't complete
#define SLANG_FAIL                        SLANG_MAKE_ERROR(GENERAL, 1)

//! Used to identify a Result that has yet to be initialized.  
//! It defaults to failure such that if used incorrectly will fail, as similar in concept to using an uninitialized variable. 
#define SLANG_E_MISC_UNINITIALIZED			SLANG_MAKE_ERROR(MISC, 2)
//! Returned from an async method meaning the output is invalid (thus an error), but a result for the request is pending, and will be returned on a subsequent call with the async handle. 			
#define SLANG_E_MISC_PENDING				SLANG_MAKE_ERROR(MISC, 3)
//! Indicates that a handle passed in as parameter to a method is invalid. 
#define SLANG_E_MISC_INVALID_HANDLE			SLANG_MAKE_ERROR(MISC, 4)

/*! Set SLANG_HANDLE_RESULT_FAIL(x) to code to be executed whenever an error occurs, and is detected by one of the macros */
#ifndef SLANG_HANDLE_RESULT_FAIL
#	define SLANG_HANDLE_RESULT_FAIL(x)
#endif

/* !!!!!!!!!!!!!!!!!!!!!!!!! Checking codes !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

//! Use to test if a result was failure. Never use result != SLANG_OK to test for failure, as there may be successful codes != SLANG_OK.
#define SLANG_FAILED(status) ((status) < 0)
//! Use to test if a result succeeded. Never use result == SLANG_OK to test for success, as will detect other successful codes as a failure.
#define SLANG_SUCCEEDED(status) ((status) >= 0)

//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result. 
#define SLANG_RETURN_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return _res; } }
//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define SLANG_RETURN_VOID_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return; } }
//! Helper macro that will return false on failure.
#define SLANG_RETURN_FALSE_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return false; } }

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