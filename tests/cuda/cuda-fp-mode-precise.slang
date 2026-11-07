// Emit-level check (no GPU): under `-fp-mode precise`, the CUDA source spells scalar
// floating-point add/sub/mul with the CUDA `_rn` intrinsics, which nvcc never contracts into an
// FMA regardless of its `--fmad` setting. Vector and matrix operators live in the #include'd
// prelude, so slangc's output carries the define that selects their `_rn` form; the prelude
// bodies themselves are covered by tests/cuda/cuda-prelude-precise-math.cu.

//TEST:SIMPLE(filecheck=PRECISE): -stage compute -entry computeMain -target cuda -fp-mode precise
//TEST:SIMPLE(filecheck=DEFAULT): -stage compute -entry computeMain -target cuda
//TEST:SIMPLE(filecheck=FAST): -stage compute -entry computeMain -target cuda -fp-mode fast
// The define and the intrinsics are CUDA-only: they must never leak into the shared C++ base
// emitter's output.
//TEST:SIMPLE(filecheck=CPP): -stage compute -entry computeMain -target cpp -fp-mode precise

RWStructuredBuffer<float> floatBuf;
RWStructuredBuffer<double> doubleBuf;
RWStructuredBuffer<half> halfBuf;
RWStructuredBuffer<int> intBuf;
RWStructuredBuffer<float3> vecBuf;
RWStructuredBuffer<float3x3> matBuf;

float mulAddFloat(float a, float b, float c) { return a * b + c; }
float mulSubFloat(float a, float b, float c) { return c - a * b; }
double mulAddDouble(double a, double b, double c) { return a * b + c; }
half mulAddHalf(half a, half b, half c) { return a * b + c; }
int mulAddInt(int a, int b, int c) { return a * b + c; }
float3 mulAddVector(float3 a, float3 b, float3 c) { return a * b + c; }
float3x3 mulAddMatrix(float3x3 a, float3x3 b, float3x3 c) { return a * b + c; }
float explicitFma(float a, float b, float c) { return fma(a, b, c); }

[Differentiable]
float differentiable(float a, float b) { return a * b + a; }

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    floatBuf[0] = mulAddFloat(floatBuf[1], floatBuf[2], floatBuf[3]);
    floatBuf[4] = mulSubFloat(floatBuf[1], floatBuf[2], floatBuf[3]);
    floatBuf[5] = explicitFma(floatBuf[1], floatBuf[2], floatBuf[3]);
    floatBuf[6] = fwd_diff(differentiable)(diffPair(floatBuf[1], 1.0), diffPair(floatBuf[2], 0.0)).d;
    doubleBuf[0] = mulAddDouble(doubleBuf[1], doubleBuf[2], doubleBuf[3]);
    halfBuf[0] = mulAddHalf(halfBuf[1], halfBuf[2], halfBuf[3]);
    intBuf[0] = mulAddInt(intBuf[1], intBuf[2], intBuf[3]);
    vecBuf[0] = mulAddVector(vecBuf[1], vecBuf[2], vecBuf[3]);
    matBuf[0] = mulAddMatrix(matBuf[1], matBuf[2], matBuf[3]);
}

// The define must precede the prelude include so the prelude's `#if` gate sees it.
//PRECISE: #define SLANG_CUDA_ENABLE_PRECISE_MATH 1
//PRECISE: slang-cuda-prelude.h
//PRECISE-LABEL: float mulAddFloat_0(
//PRECISE: return __fadd_rn(__fmul_rn(a_{{[0-9]+}}, b_{{[0-9]+}}), c_{{[0-9]+}});
//PRECISE-LABEL: float mulSubFloat_0(
//PRECISE: return __fsub_rn(c_{{[0-9]+}}, __fmul_rn(a_{{[0-9]+}}, b_{{[0-9]+}}));
// An explicit fma() is the user's request for a fused operation and stays fused.
//PRECISE-LABEL: float explicitFma_0(
//PRECISE: F32_fma(
// The precise mode is module-wide, so it also covers the derivative function that auto-diff
// generates, even though that function carries a `Fast` floating-point-mode override.
//PRECISE-LABEL: s_fwd_differentiable_0(
//PRECISE: __fadd_rn(__fmul_rn(dpa_{{[0-9]+}}->primal_0, dpb_{{[0-9]+}}->primal_0), dpa_{{[0-9]+}}->primal_0)
//PRECISE-LABEL: double mulAddDouble_0(
//PRECISE: return __dadd_rn(__dmul_rn(a_{{[0-9]+}}, b_{{[0-9]+}}), c_{{[0-9]+}});
//PRECISE-LABEL: __half mulAddHalf_0(
//PRECISE: return __hadd_rn(__hmul_rn(a_{{[0-9]+}}, b_{{[0-9]+}}), c_{{[0-9]+}});
// Integer arithmetic has no rounding to protect and keeps its infix form.
//PRECISE-LABEL: int mulAddInt_0(
//PRECISE: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};
// Vector and matrix operators keep their infix form here; the prelude implements them.
//PRECISE-LABEL: float3  mulAddVector_0(
//PRECISE: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};
//PRECISE-LABEL: Matrix<float, 3, 3>  mulAddMatrix_0(
//PRECISE: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};

//DEFAULT-NOT: SLANG_CUDA_ENABLE_PRECISE_MATH
//DEFAULT-NOT: _rn(
//DEFAULT-LABEL: float mulAddFloat_0(
//DEFAULT: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};
//DEFAULT-NOT: _rn(

//FAST-NOT: SLANG_CUDA_ENABLE_PRECISE_MATH
//FAST-NOT: _rn(
//FAST-LABEL: float mulAddFloat_0(
//FAST: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};
//FAST-NOT: _rn(

//CPP-NOT: SLANG_CUDA_ENABLE_PRECISE_MATH
//CPP-NOT: _rn(
//CPP-LABEL: float mulAddFloat_0(
//CPP: return a_{{[0-9]+}} * b_{{[0-9]+}} + c_{{[0-9]+}};
//CPP-NOT: _rn(
