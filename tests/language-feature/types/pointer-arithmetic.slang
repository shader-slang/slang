// pointer-arithmetic.slang

// DISABLED: requires GPU runtime (__getAddress on buffer elements unsupported on CPU/SPIRV)
// Test pointer arithmetic (ptr + offset) and address-of (__getAddress).

//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[100 200 300 400], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Get pointer to first element using __getAddress
    int* ptr = __getAddress(dataBuffer[0]);

    // Read through base pointer
    // CHECK: 100
    outputBuffer[0] = *ptr;

    // Pointer arithmetic: advance by 1 element
    int* ptr1 = ptr + 1;
    // CHECK: 200
    outputBuffer[1] = *ptr1;

    // Pointer arithmetic: advance by 2 elements from base
    int* ptr2 = ptr + 2;
    // CHECK: 300
    outputBuffer[2] = *ptr2;

    // Pointer arithmetic: advance by 3 elements from base
    int* ptr3 = ptr + 3;
    // CHECK: 400
    outputBuffer[3] = *ptr3;
}
