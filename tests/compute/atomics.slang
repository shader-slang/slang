// atomics.slang

//TEST(compute):COMPARE_COMPUTE(targets=all-cpu-wgpu-llvm):-shaderobj
//TEST:COMPARE_COMPUTE(targets=metal):
// Not supported in WGSL: Use of traditional atomics intrinsics (InterlockedXXX functions)
// Not supported in LLVM: atomics are not currently available

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name outputBuffer

RWStructuredBuffer<uint> outputBuffer;

void test(uint val)
{
    uint originalValue;

	InterlockedAdd(outputBuffer[val], 		val, 		originalValue);
	InterlockedAdd(outputBuffer[val ^ 1], 	val*16, 	originalValue);
	InterlockedAdd(outputBuffer[val ^ 2], 	val*16*16, 	originalValue);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    test(tid);
}
