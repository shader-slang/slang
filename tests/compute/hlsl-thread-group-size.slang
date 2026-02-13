// Test that HLSL [numthreads] produces the correct thread group size,
// verifying all three dimensions. Control test for the GLSL counterpart.
// Regression test for https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain

// Thread group: 2 x 3 x 4 = 24 threads total
// Buffer: 25 slots (24 for thread writes + 1 sentinel for overflow)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(2, 3, 4)]
void computeMain(uint tid : SV_GroupIndex)
{
    // Clamp to buffer size to detect overflow without out-of-bounds write
    uint idx = min(tid, 24);
    outputBuffer[idx] = tid + 1;

    // 24 threads write values 1..24 to slots 0..23.
    // Slot 24 stays 0, proving no overflow.
    // BUF: 1
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    // BUF-NEXT: 4
    // BUF-NEXT: 5
    // BUF-NEXT: 6
    // BUF-NEXT: 7
    // BUF-NEXT: 8
    // BUF-NEXT: 9
    // BUF-NEXT: A
    // BUF-NEXT: B
    // BUF-NEXT: C
    // BUF-NEXT: D
    // BUF-NEXT: E
    // BUF-NEXT: F
    // BUF-NEXT: 10
    // BUF-NEXT: 11
    // BUF-NEXT: 12
    // BUF-NEXT: 13
    // BUF-NEXT: 14
    // BUF-NEXT: 15
    // BUF-NEXT: 16
    // BUF-NEXT: 17
    // BUF-NEXT: 18
    // BUF-NEXT: 0
}
