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
gfx_ShaderProgram_0* loadShaderProgram_0(gfx_Renderer_0* _0, unsigned char _1[], size_t _2);
gfx_DescriptorSetLayout_0* buildDescriptorSetLayout_0(gfx_Renderer_0* _0);
gfx_PipelineLayout_0* buildPipeline_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1);
gfx_DescriptorSet_0* buildDescriptorSet_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1, gfx_BufferResource_0* _2);
gfx_PipelineState_0* buildPipelineState_0(gfx_ShaderProgram_0* _0, gfx_Renderer_0* _1, gfx_PipelineLayout_0* _2);
void dispatchComputation_0(gfx_Renderer_0* _0, gfx_PipelineState_0* _1, gfx_PipelineLayout_0* _2, gfx_DescriptorSet_0* _3, uint32_t _4, uint32_t _5, uint32_t _6);
gfx_BufferResource_0* unconvertBuffer_0(RWStructuredBuffer<float> _0);

#endif
