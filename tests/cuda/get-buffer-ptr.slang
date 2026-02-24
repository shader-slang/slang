//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj

// Test that __getStructuredBufferPtr and __getByteAddressBufferPtr work on CUDA.
// Ptr<T[]> emits as T** on CUDA, and the buffer's .data pointer is used.

//TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[100 200 300 400], stride=4):name=inputBytes
RWByteAddressBuffer inputBytes;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let sptr = __getStructuredBufferPtr(inputBuffer);
    outputBuffer[tid.x] = (*sptr)[tid.x] + 1;

    let bptr = __getByteAddressBufferPtr(inputBytes);
    outputBuffer[tid.x + 4] = int((*bptr)[tid.x]) + 2;
}

// CHECK: 11
// CHECK: 21
// CHECK: 31
// CHECK: 41
// CHECK: 102
// CHECK: 202
// CHECK: 302
// CHECK: 402
