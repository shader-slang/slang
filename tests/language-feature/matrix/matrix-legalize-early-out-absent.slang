// Regression for shader-slang/slang#11917: baseline for the `legalizeMatrixTypes` early-out. This
// shader has NO matrix that needs legalizing — a `float` matrix is emitted as a native matrix type
// on the SPIR-V / GLSL family (`shouldLowerMatrixType` is false for float elements) — so the pass's
// early-out scan finds nothing and skips the module walk. The checks below assert that the native
// matrix type SURVIVES to emission: SPIR-V keeps `OpTypeMatrix` and GLSL keeps `mat2x2`. If the pass
// ever wrongly lowered this float matrix, those types would be replaced by arrays of vectors and the
// checks would fail.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

RWStructuredBuffer<float2x2> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float2x2 m = float2x2(float(tid.x), 2, 3, 4);
    outputBuffer[tid.x] = m + m;
}

// SPV: OpTypeMatrix
// GLSL: mat2x2
