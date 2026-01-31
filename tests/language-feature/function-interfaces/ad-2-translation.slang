//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Differentiable]
float bar(float x)
{
    return 2 * x;
}

[Differentiable]
float zap(float x)
{
    // attribute->witnessCache = {bar : [bar : IBackwardDifferentiable, bar : IForwardDifferentiable, bar.BwdCallable : IBwdCallable]} 
    return bar(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    var bwdCallable = zap.apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val(); // zap(1.f);

    var dx = 0.f;

    bwdCallable(dx, 1.f); // (out float, float) -> void
    outputBuffer[tig + 1] = dx; // Expect:

    bwdCallable(dx, 3.f);
    outputBuffer[tig + 2] = dx; // Expect:
}
