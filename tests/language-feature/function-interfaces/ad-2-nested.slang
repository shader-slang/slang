//TEST:COMPARE_COMPUTE(filecheck-buffer=CHK): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

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

// TODO: Do module scope checking when deciding if we have access to zap
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
    var bwdCallable = zap.apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val(); // CHK: 2.0

    var dx = 0.f;

    bwdCallable(dx, 1.f); 
    outputBuffer[tig + 1] = dx; // CHK: 2.0

    bwdCallable(dx, 3.f);
    outputBuffer[tig + 2] = dx; // CHK: 6.0
}
