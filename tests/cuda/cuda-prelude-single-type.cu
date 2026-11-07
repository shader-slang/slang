// Compiled directly by nvcc in ci-slang-test.yml, once for each of SLANG_CUDA_ENABLE_HALF,
// _BF16 and _FP8 alone; it has no slang-test directive because slang-test's CUDA path uses
// NVRTC, which takes the prelude's SLANG_CUDA_RTC branch. -target cuda output defines only
// the enable macros its module needs (#14037), so a bf16-only or fp8-only module reaches the
// offline prelude branch without SLANG_CUDA_ENABLE_HALF. Compiling is the whole test: it fails
// if one type's section depends on another type's macro, for example SLANG_MAKE_VECTOR (which
// generates the make_* helpers below) or the FP8 half-accumulator MMA helper (which needs
// half's register counts).

#if (                                                                   \
    defined(SLANG_CUDA_ENABLE_HALF) + defined(SLANG_CUDA_ENABLE_BF16) + \
    defined(SLANG_CUDA_ENABLE_FP8)) != 1
#error "compile with exactly one of -DSLANG_CUDA_ENABLE_HALF, _BF16 or _FP8"
#endif

#include "slang-cuda-prelude.h"

static_assert(!SLANG_CUDA_RTC, "this fixture tests the prelude's offline (non-NVRTC) branch");

#if defined(SLANG_CUDA_ENABLE_HALF)
__global__ void testHalfOnly(const __half* in, __half2* out)
{
    out[0] = make___half2(in[0], in[1]);
}
#elif defined(SLANG_CUDA_ENABLE_BF16)
__global__ void testBfloat16Only(const __nv_bfloat16* in, __nv_bfloat162* out)
{
    out[0] = make___nv_bfloat162(in[0], in[1]);
}
#elif defined(SLANG_CUDA_ENABLE_FP8)
__global__ void testFp8Only(const __nv_fp8_e4m3* in, __nv_fp8_e4m32* out, __nv_fp8_e5m22* out2)
{
    out[0] = make___nv_fp8_e4m32(in[0], in[1]);
    out2[0] = make___nv_fp8_e5m22(__nv_fp8_e5m2(float(in[0])), __nv_fp8_e5m2(float(in[1])));
}
#endif
