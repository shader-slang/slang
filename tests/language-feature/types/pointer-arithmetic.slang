// Pointer arithmetic (ptr + offset) on an address taken from a mutable
// structured-buffer element via __getAddress (shader-slang/slang#12581).

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

    int* ptr1 = ptr + 1;
    // CHECK: 200
    outputBuffer[1] = *ptr1;

    int* ptr2 = ptr + 2;
    // CHECK: 300
    outputBuffer[2] = *ptr2;

    int* ptr3 = ptr + 3;
    // CHECK: 400
    outputBuffer[3] = *ptr3;
}
