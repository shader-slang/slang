//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-wgpu

// Note: getStringHash works in C++ here because 
// it is used directly on the literal
// and can be resolved at compile time without the type inlining pass
//TEST(compute):COMPARE_COMPUTE: -cpu


import string_literal_module;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(
    uint3 tid   : SV_DispatchThreadID)
{
    int value = doSomethingElse() + getStringHash("Hello \t\n\0x083 World");   
    outputBuffer[tid.x] = value;
}

