// pointer-subscript.slang

// DISABLED: requires GPU runtime (__getAddress on buffer elements unsupported on CPU/SPIRV)
// Test subscript operator on pointers (ptr[index]).

//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[11 22 33 44], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Get pointer to first element
    int* ptr = __getAddress(dataBuffer[0]);

    // Subscript with index 0 (same as *ptr)
    // CHECK: 11
    outputBuffer[0] = ptr[0];

    // Subscript with index 1
    // CHECK: 22
    outputBuffer[1] = ptr[1];

    // Subscript with index 2
    // CHECK: 33
    outputBuffer[2] = ptr[2];

    // Subscript with index 3
    // CHECK: 44
    outputBuffer[3] = ptr[3];
}
