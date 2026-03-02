//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test: Array of deeply nested struct containing ParameterBlock - should error

struct DeeplyNested
{
    struct Inner
    {
        ParameterBlock<int> deepPB;
    }
    Inner inner;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

uniform DeeplyNested arrayOfDeeplyNested[4];
//CHECK:             ^^^^^^^^^^^^^^^^^^^ arrays of non-addressable type not allowed