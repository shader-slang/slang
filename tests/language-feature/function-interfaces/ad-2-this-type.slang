//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Foo
{
    int factor;
    float zap(float x)
    {
        return factor * x;
    }
}

extension __func_as_type(Foo::zap) : IBackwardDifferentiable<__func_as_type(Foo::zap)>
{ };

extension __func_as_type(Foo::zap) : IForwardDifferentiable<__func_as_type(Foo::zap)>
{ };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    Foo foo = {2};

    var bwdCallable = foo.zap.apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val();

    var dx = 0.f;

    bwdCallable(none, dx, 1.f);
    outputBuffer[tig + 1] = dx; // Expect:

    bwdCallable(none, dx, 3.f);
    outputBuffer[tig + 2] = dx; // Expect:
}
