//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[ForwardDerivative(bar_fwd)]
float bar(float x)
{
    return 2 * x;
}

DifferentialPair<float> bar_fwd(DifferentialPair<float> dpx)
{
    return DifferentialPair<float>(2 * dpx.p, 2 * dpx.d);
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = bar.fwd_diff(DifferentialPair<float>(2.f, 1.f)).d;
}
