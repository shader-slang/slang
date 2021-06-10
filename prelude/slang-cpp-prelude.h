#ifndef SLANG_CPP_PRELUDE_H
#define SLANG_CPP_PRELUDE_H

// Because the signiture of isnan, isfinite, and is isinf changed in C++, we use the macro
// to use the version in the std namespace. 
// https://stackoverflow.com/questions/39130040/cmath-hides-isnan-in-math-h-in-c14-c11
 
#if SLANG_GCC_FAMILY && __GNUC__ < 6
#   include <cmath>
#   define SLANG_PRELUDE_STD std::
#else
#   include <math.h>
#   define SLANG_PRELUDE_STD
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

// Detect the compiler type

#ifndef SLANG_COMPILER
#    define SLANG_COMPILER

/*
Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
NOTE that SLANG_VC holds the compiler version - not just 1 or 0
*/
#    if defined(_MSC_VER)
#        if _MSC_VER >= 1900
#            define SLANG_VC 14
#        elif _MSC_VER >= 1800
#            define SLANG_VC 12
#        elif _MSC_VER >= 1700
#            define SLANG_VC 11
#        elif _MSC_VER >= 1600
#            define SLANG_VC 10
#        elif _MSC_VER >= 1500
#            define SLANG_VC 9
#        else
#            error "unknown version of Visual C++ compiler"
#        endif
#    elif defined(__clang__)
#        define SLANG_CLANG 1
#    elif defined(__SNC__)
#        define SLANG_SNC 1
#    elif defined(__ghs__)
#        define SLANG_GHS 1
#    elif defined(__GNUC__) /* note: __clang__, __SNC__, or __ghs__ imply __GNUC__ */
#        define SLANG_GCC 1
#    else
#        error "unknown compiler"
#    endif
/*
Any compilers not detected by the above logic are now now explicitly zeroed out.
*/
#    ifndef SLANG_VC
#        define SLANG_VC 0
#    endif
#    ifndef SLANG_CLANG
#        define SLANG_CLANG 0
#    endif
#    ifndef SLANG_SNC
#        define SLANG_SNC 0
#    endif
#    ifndef SLANG_GHS
#        define SLANG_GHS 0
#    endif
#    ifndef SLANG_GCC
#        define SLANG_GCC 0
#    endif
#endif /* SLANG_COMPILER */

#define SLANG_GCC_FAMILY (SLANG_CLANG || SLANG_SNC || SLANG_GHS || SLANG_GCC)

// GCC Specific
#if SLANG_GCC_FAMILY
#	define SLANG_ALIGN_OF(T)	__alignof__(T)
// Use this macro instead of offsetof, because gcc produces warning if offsetof is used on a 
// non POD type, even though it produces the correct result
#   define SLANG_OFFSET_OF(T, ELEMENT) (size_t(&((T*)1)->ELEMENT) - 1)
#endif // SLANG_GCC_FAMILY

// Microsoft VC specific
#if SLANG_VC
#   define SLANG_ALIGN_OF(T) __alignof(T)
#endif // SLANG_VC

// Default impls

#ifndef SLANG_OFFSET_OF
#   define SLANG_OFFSET_OF(X, Y) offsetof(X, Y)
#endif

#include "slang-cpp-types.h"
#include "slang-cpp-scalar-intrinsics.h"

// TODO(JS): Hack! Output C++ code from slang can copy uninitialized variables. 
#if defined(_MSC_VER)
#   pragma warning(disable : 4700)
#endif

#ifndef SLANG_UNROLL
#   define SLANG_UNROLL
#endif

struct gfx_Renderer_0;
struct gfx_BufferResource_0;
struct gfx_ShaderProgram_0;
struct gfx_DescriptorSetLayout_0;
struct gfx_PipelineLayout_0;
struct gfx_DescriptorSet_0;
struct gfx_BufferResource_0;
struct gfx_PipelineState_0;
struct gfx_CommandQueue_0;
gfx_ShaderProgram_0* loadShaderProgram_0(gfx_Renderer_0* _0, unsigned char _1[], size_t _2);
gfx_DescriptorSetLayout_0* buildDescriptorSetLayout_0(gfx_Renderer_0* _0);
gfx_PipelineLayout_0* buildPipeline_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1);
gfx_DescriptorSet_0* buildDescriptorSet_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1, gfx_BufferResource_0* _2);
gfx_PipelineState_0* buildPipelineState_0(gfx_ShaderProgram_0* _0, gfx_Renderer_0* _1, gfx_PipelineLayout_0* _2);
void dispatchComputation_0(gfx_Renderer_0* _0, gfx_PipelineState_0* _1, gfx_PipelineLayout_0* _2, gfx_DescriptorSet_0* _3, uint32_t _4, uint32_t _5, uint32_t _6);
gfx_BufferResource_0* unconvertBuffer_0(RWStructuredBuffer<float> _0);

#endif
