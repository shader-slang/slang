// This file is compiled directly by nvcc in ci-slang-test.yml. Do not add a slang-test
// directive here: slang-test's CUDA pass-through uses NVRTC, which takes the other prelude path.

#define SLANG_CUDA_ENABLE_HALF 1
#define SLANG_CUDA_ENABLE_BF16 1
#define SLANG_CUDA_ENABLE_FP8 1
#include "slang-cuda-prelude.h"

static_assert(!SLANG_CUDA_RTC, "This test must compile the offline nvcc prelude path.");

__global__ void testVec1MakeHelpers(
    const __nv_bfloat16* bfloat16Input,
    const __nv_fp8_e4m3* fp8E4M3Input,
    const __nv_fp8_e5m2* fp8E5M2Input,
    __nv_bfloat161* bfloat16Output,
    __nv_fp8_e4m31* fp8E4M3Output,
    __nv_fp8_e5m21* fp8E5M2Output)
{
    bfloat16Output[0] = make___nv_bfloat161(bfloat16Input[0]);
    fp8E4M3Output[0] = make___nv_fp8_e4m31(fp8E4M3Input[0]);
    fp8E5M2Output[0] = make___nv_fp8_e5m21(fp8E5M2Input[0]);
}
