#ifndef SLANG_CPP_PRELUDE_H
#define SLANG_CPP_PRELUDE_H

#include "../slang.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
//#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__ ((dllexport)) __attribute__((__visibility__("default")))
#endif    

#ifdef __cplusplus    
#   define SLANG_PRELUDE_EXTERN_C extern "C"
#else
#   define SLANG_PRELUDE_EXTERN_C 
#endif    

#define SLANG_PRELUDE_EXPORT SLANG_PRELUDE_EXTERN_C SLANG_PRELUDE_SHARED_LIB_EXPORT

#ifndef SLANG_INFINITY
#   define SLANG_INFINITY   INFINITY
#endif

#include "slang-cpp-types.h"
#include "slang-cpp-scalar-intrinsics.h"

// TODO(JS): Hack! Output C++ code from slang can copy uninitialized variables. 
#if SLANG_VC
#   pragma warning(disable : 4700)
#endif

#ifndef SLANG_UNROLL
#   define SLANG_UNROLL
#endif

#endif
