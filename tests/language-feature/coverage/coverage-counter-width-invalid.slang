// Verifies that `-trace-coverage-counter-width` rejects values
// other than 32 or 64 with a clear diagnostic. The flag accepts
// exactly the two byte widths that the IR coverage pass and the
// LCOV converter support; any other value would either silently
// fall back to one of them or produce a buffer layout that no
// downstream tool can read.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 16

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

//CHECK: E45113
