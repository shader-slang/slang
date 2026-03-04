//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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

uniform StructWithParameterBlock arrayOfStructWithPB[3];
//CHECK:                         ^^^^^^^^^^^^^^^^^^^ arrays of non-addressable type not allowed
//CHECK:                         ^^^^^^^^^^^^^^^^^^^ Arrays of non-addressable type 'StructWithParameterBlock' are not allowed
//CHECK:                         ^^^^^^^^^^^^^^^^^^^ arrays of non-addressable type not allowed
//CHECK:                         ^^^^^^^^^^^^^^^^^^^ Arrays of non-addressable type 'StructWithParameterBlock' are not allowed