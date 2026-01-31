//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

float zap(float x)
{
    return 2 * x;
}

extension __func_as_type(zap) : IBackwardDifferentiable<__func_as_type(zap)>
{ };

extension __func_as_type(zap) : IForwardDifferentiable<__func_as_type(zap)>
{ };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    var bwdCallable = zap.apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val();

    var dx = 0.f;

    bwdCallable(dx, 1.f);
    outputBuffer[tig + 1] = dx; // Expect:

    bwdCallable(dx, 3.f);
    outputBuffer[tig + 2] = dx; // Expect:
}
