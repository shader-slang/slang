//TEST:SIMPLE(filecheck=CHECK_FAIL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=16):out
RWStructuredBuffer<float> outputBuffer;

// CHECK_FAIL-DAG: ([[#@LINE+2]]): note:{{.*}}testABwd
[require(spirv)]
void testABwd(inout DifferentialPair<float> x, float result)
{
    x = {x.p, 0};
}

[BackwardDerivative(testABwd)]
float testA(float x)
{
   return x*2;
}

[BackwardDifferentiable]
float wrapper()
{
    return testA(1.0f);
}

// CHECK_PASS-NOT: error 36107
// CHECK_FAIL-DAG: ([[#@LINE+3]]): error 36107
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    bwd_diff(wrapper)(1.0f);
}