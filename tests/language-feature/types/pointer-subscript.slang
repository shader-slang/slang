// Subscript (ptr[index]) on a pointer taken from a mutable structured-buffer
// element via __getAddress (shader-slang/slang#12581).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[11 22 33 44], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* ptr = __getAddress(dataBuffer[0]);

    // CHECK: 11
    outputBuffer[0] = ptr[0];

    // CHECK: 22
    outputBuffer[1] = ptr[1];

    // CHECK: 33
    outputBuffer[2] = ptr[2];

    // CHECK: 44
    outputBuffer[3] = ptr[3];
}
