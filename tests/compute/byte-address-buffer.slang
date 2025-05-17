// byte-address-buffer.slang

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj

//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Confirm cross-compilation of `(RW)ByteAddressBuffer`
//
// TODO: I'm only using `RWByteAddressBuffer` for now because I don't
// know if `render-test` supports the non-UAV case.

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15]):name=inputBuffer
RWByteAddressBuffer inputBuffer;

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
