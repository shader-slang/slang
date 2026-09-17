//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(smoke):SIMPLE:

// CHECK: type: float
// CHECK-NEXT: 5.500000

//TEST_INPUT:ubuffer(format=R_Float32, data=[1.25 2.5 4.0], stride=4):name=inputBuffer
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer

T readAt<T, TArray : IArray<T>>(TArray values, uint index)
{
    return values[index];
}

Buffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readAt(inputBuffer, 1) + float(inputBuffer.getCount());
}
