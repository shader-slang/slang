// atomics-buffer.slang

// TODO: Remove `glsl` from the targets when https://github.com/shader-slang/slang/issues/14592 is fixed. The Vulkan validation layer
// rejects the buffer view (VUID-VkBufferViewCreateInfo-range-00929).
//TEST:COMPARE_COMPUTE(targets=all-vk-glsl-metal-cuda-cpu-wgpu-llvm):
// Doesn't work on VK - GLSL output doesn't replace InterlockedAdd.
// Cannot work on CUDA, as outputBuffer becomes a CUsurfObject - which do not appear to have atomics available.
// If the buffer was a StructuredBuffer this would work on CUDA.
// Atomics not available on CPU currently
// RWBuffer does not work with the GFX backend as expected with Metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
// Not supported in WGSL: Use of traditional atomics intrinsics (InterlockedXXX functions)
// Not supported in LLVM: atomics are not currently available

//METALLIB: @computeMain

//TEST_INPUT:ubuffer(format=R_UInt32, data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]):out,name outputBuffer

RWBuffer<uint> outputBuffer;

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
