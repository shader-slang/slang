//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0], stride=16):out
RWStructuredBuffer<float> outputBuffer;

//CHECK:error 36107

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

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float x = 2.0;
    var dx = diffPair(x);

    bwd_diff(testA)(dx, 1.0);
    outputBuffer[0] = dx.d;
}