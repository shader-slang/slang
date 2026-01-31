//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Differentiable]
void bar(float x, out float y)
{
    y = 2 * x;
}

[Differentiable]
void zap(float x, out float y)
{
    return bar(x, y);
}

extension __func_as_type(zap) : IBackwardDifferentiable<__func_as_type(zap)>
{ };

extension __func_as_type(zap) : IForwardDifferentiable<__func_as_type(zap)>
{ };

extension __func_as_type(bar) : IBackwardDifferentiable<__func_as_type(bar)>
{ };

extension __func_as_type(bar) : IForwardDifferentiable<__func_as_type(bar)>
{ };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    float primal;
    var bwdCallable = zap.apply_bwd(1.f, primal);
    //outputBuffer[tig] = bwdCallable.val();

    var dx = 0.f;

    bwdCallable(dx, 1.f);
    outputBuffer[tig + 1] = dx; // Expect:

    bwdCallable(dx, 3.f);
    outputBuffer[tig + 2] = dx; // Expect:
}
