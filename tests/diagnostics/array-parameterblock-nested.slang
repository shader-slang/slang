//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test: Array of nested struct containing ParameterBlock - should error

struct Inner
{
    ParameterBlock<float> pb;
}

struct NestedStruct
{
    Inner nested;
    float value;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Force usage of the array to prevent optimization
    outputBuffer[dispatchThreadID.x] = arrayOfNestedStruct[dispatchThreadID.x % 2].nested.pb;
}

uniform NestedStruct arrayOfNestedStruct[2];
//CHECK:             ^^^^^^^^^^^^^^^^^^^ arrays of non-addressable type not allowed