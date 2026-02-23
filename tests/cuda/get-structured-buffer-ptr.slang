//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type -shaderobj

// Test that __getStructuredBufferPtr works on CUDA.
// Ptr<T[]> collapses to T* on CUDA, and GetStructuredBufferPtr returns the data pointer.

//TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let ptr = __getStructuredBufferPtr(inputBuffer);
    outputBuffer[tid.x] = (*ptr)[tid.x] + 1;
}

// CHECK: 11
// CHECK: 21
// CHECK: 31
// CHECK: 41
