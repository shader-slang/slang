// Regression test for shader-slang/slang#12581: `__getAddress(buf[i])` on a mutable
// structured buffer must be accepted (the element is a device-memory lvalue, and
// `&buf[i]` already compiled). Exercises reading, arithmetic, and writing through it.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[100 200 300 400], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* ptr = __getAddress(dataBuffer[0]);

    // CHECK: 100
    outputBuffer[0] = *ptr;

    // CHECK: 300
    outputBuffer[1] = *(ptr + 2);

    *ptr = 42;
    // CHECK: 42
    outputBuffer[2] = dataBuffer[0];

    int* ptr3 = __getAddress(dataBuffer[3]);
    // CHECK: 400
    outputBuffer[3] = *ptr3;
}
