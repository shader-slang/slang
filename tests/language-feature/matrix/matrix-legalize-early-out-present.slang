// Regression for shader-slang/slang#11917: `legalizeMatrixTypes` gained an early-out that skips the
// whole pass when no matrix needs legalizing. This shader DOES need it: an `int` matrix must be
// lowered to arrays of vectors on the SPIR-V / GLSL family, so the early-out's presence scan must
// detect the lowerable matrix and let the pass run. The checks assert the lowering actually
// happened — the integer matrix type must NOT survive to emission (no `OpTypeMatrix` in SPIR-V, no
// `imat` in GLSL). If the scan wrongly skipped the pass, the un-lowered integer matrix type would
// reach the emitter and these negative checks would fail (independent of SPIR-V validation).
//
// The behavioural (value) correctness of integer-matrix lowering is already covered by the GPU
// tests in tests/spirv/matrix-integer-lowering.slang etc.; this is a GPU-free emission guard.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

RWStructuredBuffer<int2x2> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int2x2 m = int2x2(int(tid.x), 2, 3, 4);
    outputBuffer[tid.x] = m + m;
}

// SPV: OpEntryPoint
// SPV-NOT: OpTypeMatrix
// GLSL: void main(
// GLSL-NOT: imat
