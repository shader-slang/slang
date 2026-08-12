//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// Regression test for comparison operators between float matrices.
//
// Comparing two float matrices (e.g. `float4x4 == float4x4`) yields a bool
// matrix result. On targets that lower bool/int/uint matrices to arrays of
// vectors (SPIR-V, GLSL, Metal, WGSL), the *result* bool matrix needs lowering
// while the float *operands* do not. The matrix-type legalization pass in
// source/slang/slang-ir-legalize-matrix-types.cpp dispatches into
// `legalizeComparisonOperation` whenever the result OR an operand needs
// lowering (see the `getReplacement` check), but previously asserted that
// *both operands* needed lowering. With float operands that assert fired in
// debug builds, and the emitted `SLANG_ASSUME` corrupted release builds,
// crashing slangc. This exercises the one-sided lowering path to confirm it
// legalizes without crashing.
//
// The operands are read from buffers so the comparisons are not constant-folded
// away; that keeps the per-row vector comparison ops live in the emitted SPIR-V.

RWStructuredBuffer<float4x4> inA;
RWStructuredBuffer<float4x4> inB;
RWStructuredBuffer<bool4x4> res4;

RWStructuredBuffer<float2x2> inC;
RWStructuredBuffer<float2x2> inD;
RWStructuredBuffer<bool2x2> res2;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // The original repro: `float4x4 == float4x4`, producing a bool4x4 result.
    // CHECK: OpFOrdEqual
    res4[0] = (inA[0] == inB[0]);

    // A relational comparison at a different matrix size exercises the same
    // `legalizeComparisonOperation` row loop for the other comparison ops.
    // CHECK: OpFOrdLessThan
    res2[0] = (inC[0] < inD[0]);
}
