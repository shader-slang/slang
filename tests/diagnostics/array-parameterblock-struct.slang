//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test: Array of struct containing ParameterBlock - should error

struct StructWithParameterBlock
{
    ParameterBlock<float> pb;
    int other;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Force usage of the array to prevent optimization
    outputBuffer[dispatchThreadID.x] = arrayOfStructWithPB[dispatchThreadID.x % 3].pb;
}

//CHECK: ([[# @LINE+1]]): error 30027
uniform StructWithParameterBlock arrayOfStructWithPB[3];