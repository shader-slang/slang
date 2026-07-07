// Regression for shader-slang/slang#11917: baseline for the `legalizeMatrixTypes` early-out. This
// shader has NO matrix that needs legalizing — a `float` matrix is emitted as a native matrix type
// on the SPIR-V / GLSL family (`shouldLowerMatrixType` is false for float elements) — so the pass's
// early-out scan finds nothing and skips the module walk. Emission must still succeed and be
// unchanged; the revert-drill (see the PR description) proves the skip is byte-identical to running
// the pass as a no-op.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float2x2 m = float2x2(1, 2, 3, 4);
    float2x2 n = m + m;
    outputBuffer[0] = n[1][1];
}

// SPV: OpEntryPoint
// GLSL: void main(
