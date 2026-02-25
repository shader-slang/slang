// Test that GLSL layout(local_size_x/y/z) produces the correct thread group
// size, verifying all three dimensions.
// Regression test for https://github.com/shader-slang/slang/issues/9953

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -allow-glsl

#version 430

// Thread group: 2 x 3 x 4 = 24 threads total
// Buffer: 25 slots (24 for thread writes + 1 sentinel for overflow)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2 { uint data[]; } outputBuffer;

layout(local_size_x = 2, local_size_y = 3, local_size_z = 4) in;

void computeMain()
{
    // Use gl_LocalInvocationID instead of gl_LocalInvocationIndex to avoid
    // a pre-existing Metal code gen issue with the built-in linearized index.
    uvec3 id = gl_LocalInvocationID;
    uint tid = id.z * 6u + id.y * 2u + id.x;
    // Clamp to buffer size to detect overflow without out-of-bounds write
    uint idx = min(tid, uint(24));
    outputBuffer.data[idx] = tid + 1;

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
