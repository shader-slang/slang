//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test: Direct array of ParameterBlock - should error

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

//CHECK: ([[# @LINE+1]]): error 30027
uniform ParameterBlock<float> qs[2];