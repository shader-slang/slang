//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float bar(float x)
{
    return 2 * x; 
}

[BackwardDerivativeOf(bar)]
void bar_bwd(inout DifferentialPair<float> dpx, float x)
{
    dpx = diffPair(2.0, 1.f);
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    let barBwdPass = bar.apply_bwd(2.f);
    float dx = 0.f;
    barBwdPass(dx, 1.f);
    outputBuffer[tig] = dx;
}
