// Compiled directly by nvcc in ci-slang-test.yml; no slang-test directive
// on purpose (slang-test's CUDA path uses NVRTC, the other prelude branch).
// Compiling is the whole test. Guarded regressions:
//   1. a vec1 make helper returning the scalar instead of T1, a type error at
//      the assignments in testVec1MakeHelpers below;
//   2. RayDesc missing outside the OptiX guard (#12638) — this fixture does not
//      define SLANG_CUDA_ENABLE_OPTIX, so testRayDescIsDefinedWithoutOptiX only
//      compiles if RayDesc lives in the always-compiled prelude region. slang-test
//      cannot catch this: its -target ptx lane runs through NVRTC and is silently
//      ignored on tiers without it.
//   3. Fast-math transcendental redirects (#12619) — this fixture defines
//      SLANG_CUDA_ENABLE_FAST_MATH, so testFastMathWrappers only compiles if the
//      gated `__*f` intrinsic names/arities are correct. This is dedicated
//      offline-nvcc coverage of the gated bodies that runs regardless of whether
//      slang-test has an NVRTC tier available (the cuda-fp-mode-fast.slang PTX lanes
//      also exercise the redirect through NVRTC, but auto-skip where NVRTC is absent).
// __half is included to pin the known-good sibling pattern.

#define SLANG_CUDA_ENABLE_HALF 1
#define SLANG_CUDA_ENABLE_BF16 1
#define SLANG_CUDA_ENABLE_FP8 1
#define SLANG_CUDA_ENABLE_FAST_MATH 1
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

__global__ void testRayDescIsDefinedWithoutOptiX(float3* out)
{
    RayDesc ray;
    ray.Origin = make_float3(1.0f, 2.0f, 3.0f);
    ray.TMin = 4.0f;
    ray.Direction = make_float3(5.0f, 6.0f, 7.0f);
    ray.TMax = 8.0f;
    out[0] = ray.Origin;
    out[1] = ray.Direction;
    out[2] = make_float3(ray.TMin, ray.TMax, 0.0f);
}

// With SLANG_CUDA_ENABLE_FAST_MATH defined above, this instantiates the fast branch
// of every gated wrapper, so a wrong `__*f` name or arity fails to compile here.
// It also references F16_tan/F16_pow (which reroute through F32_tan/F32_pow) and a
// representative sample of the un-redirected wrappers (F32_exp2, F64_sin, F64_exp).
__global__ void testFastMathWrappers(float* fout, __half* hout, double* dout)
{
    float x = fout[0];
    float s, c;
    F32_sincos(x, &s, &c);
    fout[0] = F32_sin(x) + F32_cos(x) + s + c + F32_tan(x) + F32_log(x) + F32_log2(x) +
              F32_log10(x) + F32_exp(x) + F32_pow(x, 2.0f) + F32_exp2(x);

    __half h = hout[0];
    hout[0] = F16_tan(h) + F16_pow(h, h);

    double d = dout[0];
    dout[0] = F64_sin(d) + F64_exp(d);
}
