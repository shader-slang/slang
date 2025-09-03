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
    {
        DifferentialPair<float> dpx = diffPair(1.f, 0.f);
        bwd_diff(bar)(dpx, 1.f);
        outputBuffer[tig] = dpx.d;
    }

    {
        DifferentialPair<float> dpx = diffPair(2.f, 0.f);
        bar.bwd_diff(dpx, 2.f);
        outputBuffer[tig + 1] = dpx.d;
    }

    {
        let barBwdPass = bar.apply_bwd(2.f);
        float dx = 0.f;
        barBwdPass(dx, 1.f);
        outputBuffer[tig + 2] = dx;
    }
}
