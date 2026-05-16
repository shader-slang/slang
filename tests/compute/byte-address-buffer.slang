// byte-address-buffer.slang

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj -xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -xslang -DRWBAB
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj -xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj -xslang -DRWBAB

//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Confirm cross-compilation of `ByteAddressBuffer` and `RWByteAddressBuffer`.

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15]):name=inputBuffer
#ifdef RWBAB
RWByteAddressBuffer inputBuffer;
#else
ByteAddressBuffer inputBuffer;
#endif

//TEST_INPUT:ubuffer(data=[0 0 0 0]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

// Make sure arithmetic ops are properly performed on unsigned types.
// SPV-NOT: OpSDiv
// SPV: OpUDiv

void test(uint val)
{
	uint tmp = val;

	tmp = inputBuffer.Load(uint(tmp * 4));

	uint2 pair = inputBuffer.Load2(uint(tmp * 4));
	tmp = (pair.x + pair.y) & 0xF;

	uint4 quad = inputBuffer.Load4(uint(tmp * 4));
	tmp = (quad.x + quad.y + quad.z + quad.w) & 0xF;

    uint coverage = 0;

    uint3 triple = inputBuffer.Load3(0);
    coverage += triple.x + triple.y + triple.z;

    uint2 alignedPair = inputBuffer.Load2Aligned(8, 8);
    coverage += alignedPair.x + alignedPair.y;

    uint3 alignedTriple = inputBuffer.Load3Aligned(16, 16);
    coverage += alignedTriple.x + alignedTriple.y + alignedTriple.z;

    uint4 alignedQuad = inputBuffer.Load4Aligned(16, 16);
    coverage += alignedQuad.x + alignedQuad.y + alignedQuad.z + alignedQuad.w;

    uint2 naturalPair = inputBuffer.Load2Aligned(16);
    coverage += naturalPair.x + naturalPair.y;

    uint3 naturalTriple = inputBuffer.Load3Aligned(16);
    coverage += naturalTriple.x + naturalTriple.y + naturalTriple.z;

    uint4 naturalQuad = inputBuffer.Load4Aligned(16);
    coverage += naturalQuad.x + naturalQuad.y + naturalQuad.z + naturalQuad.w;

    int genericInt = inputBuffer.Load<int>(0);
    coverage += uint(genericInt);

    float genericFloat = inputBuffer.Load<float>(4);
    coverage += asuint(genericFloat);

    float4 genericFloatQuad = inputBuffer.Load<float4>(0);
    coverage += asuint(genericFloatQuad.x);

    float4 alignedFloatQuad = inputBuffer.LoadAligned<float4>(16, 16);
    coverage += asuint(alignedFloatQuad.x);

    float4 naturalFloatQuad = inputBuffer.LoadAligned<float4>(16);
    coverage += asuint(naturalFloatQuad.x);

    // Keep the shared expected output while making coverage mismatches visible.
    tmp = coverage == 100 ? tmp : 0xFFFFFFFF;

	outputBuffer.Store(val*4, tmp);
}

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    uint val = uint(tid);
    test(val);
}
