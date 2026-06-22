// Regression test for shader-slang/slang issue #11673 (fixed by PR #11668).
//
// Comparing two `float` matrices on a target that lowers bool/int/uint matrices to
// arrays-of-vectors (SPIR-V here) is a "one-sided" matrix-lowering case: the comparison
// result is a `bool` matrix, which needs lowering, while the `float` operands do not.
// The matrix legalization pass (legalizeComparisonOperation in
// source/slang/slang-ir-legalize-matrix-types.cpp) used to assert that *both* operands
// needed lowering, so a float-matrix comparison crashed slangc (the assert fires in debug
// builds, and the SLANG_ASSUME it lowers to corrupts release builds). PR #11668 relaxes
// the assert to only require matrix-matrix operands, after which the float operands are
// legalized per row and the comparison is emitted normally. No existing test covered
// matrix comparisons; the closest, tests/legalization/matrix-cast-lowering.slang, covers
// matrix casts only.

//TEST:SIMPLE(filecheck=CHECK_EQ): -target spirv-asm -entry mainEqual -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_LT): -target spirv-asm -entry mainLess -stage compute -emit-spirv-directly

// `float4x4 == float4x4` -> `bool4x4` is the exact shape from the bug report: the bool
// result matrix is lowered while the float operands are not, exercising the one-sided
// path. Operands are loaded from buffers so the comparison survives to code emission.
RWStructuredBuffer<float4x4> inA4;
RWStructuredBuffer<float4x4> inB4;
RWStructuredBuffer<bool4x4> resEq;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainEqual()
{
    // Legalization decomposes the comparison into one per-row float-vector compare; the
    // float `==` must survive as an ordered floating-point equality in the emitted SPIR-V.
    // CHECK_EQ: OpFOrdEqual
    resEq[0] = (inA4[0] == inB4[0]);
}

// A relational comparison at a different matrix size: `float3x3 < float3x3` -> `bool3x3`.
RWStructuredBuffer<float3x3> inA3;
RWStructuredBuffer<float3x3> inB3;
RWStructuredBuffer<bool3x3> resLt;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainLess()
{
    // CHECK_LT: OpFOrdLessThan
    resLt[0] = (inA3[0] < inB3[0]);
}
