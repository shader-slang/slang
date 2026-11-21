// Read every bit of a uint4 ballot with subgroupBallotBitExtract, for indices 0 to 127, and check
// that the bits give the ballot value again. CUDA and HLSL compute the bit in the GLSL module
// (`glsl.meta.slang`) instead of with a native operation, so their result is defined for any
// index; this test checks that each of the four components is read for its 32 indices. There is
// no Vulkan line, because the GLSL spec makes an index at or above gl_SubgroupSize undefined.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj -render-feature hardware-device -output-using-type

import glsl;

// [0..3]: the ballot value; each component has a different bit pattern.
// [4]:    0, the first index; it comes from the buffer so that the indices are not constants.
//TEST_INPUT:ubuffer(data=[0x8A5D0009 0x3C96A5C3 0x0F0F00FF 0xC0000003 0], stride=4):name inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    uint4 a = uint4(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    uint4 r = uint4(0);
    for (uint i = inputBuffer[4]; i < 128; i++)
    {
        if (subgroupBallotBitExtract(a, i))
            r[i / 32] |= 1u << (i % 32);
    }
    outputBuffer[0] = r.x;
    outputBuffer[1] = r.y;
    outputBuffer[2] = r.z;
    outputBuffer[3] = r.w;
}

// The ballot value, in decimal: 0x8A5D0009, 0x3C96A5C3, 0x0F0F00FF, 0xC0000003.
// BUF: 2321350665
// BUF-NEXT: 1016505795
// BUF-NEXT: 252641535
// BUF-NEXT: 3221225475
