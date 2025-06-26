//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Differentiable]
void bar(inout float x)
{
    x = 2 * x;
}

[Differentiable]
void zap(inout float y)
{
    bar(y);
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
    float val = 1.f;
    var bwdCallable = zap.apply_bwd(val);
    //outputBuffer[tig] = bwdCallable.val();

    var dx = 1.f;

    bwdCallable(dx);
    outputBuffer[tig + 1] = dx; // Expect:

    dx = 2.f;
    bwdCallable(dx);
    outputBuffer[tig + 2] = dx; // Expect:
}