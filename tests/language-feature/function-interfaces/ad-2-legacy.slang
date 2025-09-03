//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Differentiable]
float bar(float x)
{
    return 2 * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    DifferentialPair<float> dpx = diffPair(2.f);
    bar.bwd_diff(dpx, 1.0);
    outputBuffer[tig] = dpx.d; // Expect: 2
}
