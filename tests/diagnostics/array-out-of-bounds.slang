// array-out-of-bounds.slang

// Test that out-of-bounds array access with constant indices generates an error

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a[3];
    a[0] = 10;
    a[1] = 20;
    a[2] = 30;
    
    // Valid access - should be fine
    outputBuffer[0] = a[0];
    outputBuffer[1] = a[2];
    
    // Invalid access - index 3 is out of bounds for array of size 3
    //CHECK: error 30029: array index '3' is out of bounds for array of size '3'.
    outputBuffer[2] = a[3];
    
    // Invalid access - negative index
    //CHECK: error 30029: array index '-1' is out of bounds for array of size '3'.
    outputBuffer[3] = a[-1];
}