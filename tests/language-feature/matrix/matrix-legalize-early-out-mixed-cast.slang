// Regression for shader-slang/slang#11917: exercises the exact soundness rationale of the
// `legalizeMatrixTypes` early-out — a matrix cast whose RESULT type needs no lowering but whose
// OPERAND type does. Here `y = (float2x2)x` has a `float2x2` result (native on SPIR-V/GLSL, not
// lowered) produced from a `uint2x2` operand (must be lowered to arrays of vectors). The presence
// scan `hasAnyMatrixToLegalize` keys on matrix TYPES rather than instruction results precisely so it
// still finds the lowerable `uint2x2` here (it is a hoisted global type inst) and lets the pass run.
//
// The checks assert: the native `float2x2` result SURVIVES (SPIR-V keeps `OpTypeMatrix`, GLSL keeps
// `mat2x2`), while the `uint2x2` operand is LOWERED away (no `umat`/`imat` integer matrix type in the
// GLSL output). A future refactor that keyed the scan off result types instead of operand types would
// ship an un-lowered `uint2x2` and fail the `GLSL-NOT: umat` check — a case the GPU-only cast tests
// under tests/legalization/ cannot catch in GPU-free CI.
//
// The `GLSL-NOT` checks follow the surviving-`mat2x2` positive with no positive after them, so their
// scan region runs from that match to the end of the output — it covers `main`'s body, where an
// un-lowered `uint2x2` operand would print as `umat2x2`. (Only the `#version`/`layout` preamble ahead
// of `mat2x2` is outside the region, and no matrix type is ever declared there.)

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

RWStructuredBuffer<float2x2> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2x2 x = uint2x2(tid.x, 0, 0, tid.x);
    float2x2 y = (float2x2)x;
    outputBuffer[tid.x] = y;
}

// SPV: OpTypeMatrix

// GLSL: mat2x2
// GLSL-NOT: umat
// GLSL-NOT: imat
