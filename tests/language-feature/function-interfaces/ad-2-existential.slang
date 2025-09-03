//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    static float zap(float x) : IBackwardDifferentiable<__func_as_type(This.zap)>;
};

struct FooImpl : IFoo
{
    static float zap(float x)
    {
        return 2 * x;
    }
};

struct BwdCallableForZap : IBwdCallable<__func_as_type(FooImpl::zap)>
{
    float x;
    float val() { return FooImpl::zap(x); }
    void operator()(out float.Differential dx, float.Differential dOut)
    {
        dx = 2 * x * dOut;
    }
};

extension __func_as_type(FooImpl::zap) : IBackwardDifferentiable<__func_as_type(FooImpl::zap)>
{
    typealias BwdCallable = BwdCallableForZap;
    static This.BwdCallable apply_bwd(float x)
    {
        return BwdCallableForZap(2 * x);
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IFoo foo = FooImpl();
    var bwdCallable = foo.zap.apply_bwd(1.f);
    outputBuffer[tig] = bwdCallable.val();

    var dx = 0.f;
    bwdCallable(dx, 2.0);
    outputBuffer[tig + 1] = dx; // Expect: 4.0

    bwdCallable(dx, 3.0);
    outputBuffer[tig + 2] = dx; // Expect: 6.0
}
