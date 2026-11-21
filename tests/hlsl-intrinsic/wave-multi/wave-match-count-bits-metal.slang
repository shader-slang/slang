//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type

// Runs the Metal emulation of WaveMatch, WaveMultiPrefixCountBits and the bool and matrix
// WaveActiveAllEqual on 8 threads, and checks each result in the shader. Each thread writes 1023
// (bits 0-9) when every check passes:
// - 1, 2, 4, 8: WaveMatch of a uint, a bool, a float2 and a float2x2 that are equal on the even
//   threads and on the odd threads: 0x55 on even threads, 0xAA on odd ones;
// - 16: WaveMatch of a value that differs on every thread: only the thread's own bit;
// - 32: WaveMultiPrefixCountBits(bool(i & 5), lanes 4-7): the lanes of 4-7 below this one whose
//   value is true. Lanes 4-7 are all true, so the counts are 0 0 0 0 0 1 2 3;
// - 64, 128: WaveActiveAllEqual of a bool that is equal everywhere, and of one that is not;
// - 256, 512: WaveActiveAllEqual of a float2x2 that is equal everywhere, and of one whose last
//   element differs.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;
    const uint odd = i & 1;
    const uint expectedMatch = odd != 0 ? 0xAAu : 0x55u;
    const uint expectedCount[8] = { 0, 0, 0, 0, 0, 1, 2, 3 };

    // Every SIMD-group call runs unconditionally, before any check.
    const uint4 matchUInt = WaveMatch(odd);
    const uint4 matchBool = WaveMatch(odd != 0);
    const uint4 matchVector = WaveMatch(float2(float(odd), 1.0));
    const uint4 matchMatrix = WaveMatch(float2x2(float(odd), 1.0, 2.0, 3.0));
    const uint4 matchSelf = WaveMatch(i);
    const uint count = WaveMultiPrefixCountBits((i & 5) != 0, uint4(0xF0, 0, 0, 0));
    const bool boolEqual = WaveActiveAllEqual(true);
    const bool boolDiffers = WaveActiveAllEqual(odd != 0);
    const bool matrixEqual = WaveActiveAllEqual(float2x2(1.0, 2.0, 3.0, 4.0));
    const bool matrixDiffers = WaveActiveAllEqual(float2x2(1.0, 2.0, 3.0, float(i)));

    uint result = 0;
    if (all(matchUInt == uint4(expectedMatch, 0, 0, 0)))
        result |= 1;
    if (all(matchBool == uint4(expectedMatch, 0, 0, 0)))
        result |= 2;
    if (all(matchVector == uint4(expectedMatch, 0, 0, 0)))
        result |= 4;
    if (all(matchMatrix == uint4(expectedMatch, 0, 0, 0)))
        result |= 8;
    if (all(matchSelf == uint4(1u << i, 0, 0, 0)))
        result |= 16;
    if (count == expectedCount[i])
        result |= 32;
    if (boolEqual)
        result |= 64;
    if (!boolDiffers)
        result |= 128;
    if (matrixEqual)
        result |= 256;
    if (!matrixDiffers)
        result |= 512;

    outputBuffer[i] = result;
}

// CHECK: type: uint32_t
// CHECK-COUNT-8: 1023
