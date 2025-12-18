//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

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

//CHECK: ([[# @LINE+1]]): error 30027
uniform DeeplyNested arrayOfDeeplyNested[4];