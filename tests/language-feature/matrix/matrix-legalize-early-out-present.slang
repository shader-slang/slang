// Regression for shader-slang/slang#11917: `legalizeMatrixTypes` gained an early-out that skips the
// whole pass when no matrix needs legalizing. This shader DOES need it: an `int` matrix must be
// lowered to arrays of vectors on the SPIR-V / GLSL family, so the early-out's presence scan must
// detect the lowerable matrix and let the pass run. If the scan wrongly skipped, an un-lowered
// integer matrix would reach the emitter and fail — so successful emission here is the guard.
//
// The behavioural (value) correctness of integer-matrix lowering is already covered by the GPU
// tests in tests/spirv/matrix-integer-lowering.slang etc.; this is a GPU-free emission guard.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int2x2 m = int2x2(1, 2, 3, 4);
    int2x2 n = m + m;
    outputBuffer[0] = n[1][1];
}

// SPV: OpEntryPoint
// GLSL: void main(
