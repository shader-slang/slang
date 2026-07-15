// Compiled directly by nvcc in ci-slang-test.yml; no slang-test directive
// on purpose (slang-test's CUDA path uses NVRTC, the other prelude branch).
// Compiling is the whole test: the guarded regression is a vec1 make helper
// returning the scalar instead of T1, a type error at the assignments below.
// __half is included to pin the known-good sibling pattern.

#define SLANG_CUDA_ENABLE_HALF 1
#define SLANG_CUDA_ENABLE_BF16 1
#define SLANG_CUDA_ENABLE_FP8 1
#include "slang-cuda-prelude.h"

static_assert(
    !SLANG_CUDA_RTC,
    "this fixture must be compiled with offline nvcc, not NVRTC; the vec1 "
    "make helpers under test exist only in the non-RTC prelude branch");

__global__ void testVec1MakeHelpers(
    const __half* halfInput,
    const __nv_bfloat16* bfloat16Input,
    const __nv_fp8_e4m3* fp8E4M3Input,
    const __nv_fp8_e5m2* fp8E5M2Input,
    __half1* halfOutput,
    __nv_bfloat161* bfloat16Output,
    __nv_fp8_e4m31* fp8E4M3Output,
    __nv_fp8_e5m21* fp8E5M2Output)
{
    halfOutput[0] = make___half1(halfInput[0]);
    bfloat16Output[0] = make___nv_bfloat161(bfloat16Input[0]);
    fp8E4M3Output[0] = make___nv_fp8_e4m31(fp8E4M3Input[0]);
    fp8E5M2Output[0] = make___nv_fp8_e5m21(fp8E5M2Input[0]);
}
