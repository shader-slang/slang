//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test: Direct array of ParameterBlock - should error

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

uniform ParameterBlock<float> qs[2];
//CHECK:                      ^^ arrays of non-addressable type not allowed