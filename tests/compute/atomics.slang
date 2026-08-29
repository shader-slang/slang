// atomics.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -
// Not supported in WGSL: Use of traditional atomics intrinsics (InterlockedXXX functions)
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -wgpu
// Not supported in LLVM: atomics are not currently available
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -llvm

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name outputBuffer

RWStructuredBuffer<uint> outputBuffer;

// NVVM: .visible .entry computeMain
// NVVM-COUNT-3: atom.global.add.u32

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
