//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

float zap(float x)
{
    return 2 * x;
}

struct BwdCallableForZap : IBwdCallable<__func_as_type(zap)>
{
    float x;
    float val() { return zap(x); }
    void operator()(out float.Differential dx, float.Differential dOut)
    {
        dx = 2 * x * dOut;
    }
};

extension __func_as_type(zap) : IBackwardDifferentiable<__func_as_type(zap)>
{
    typealias BwdCallable = BwdCallableForZap;
    static This.BwdCallable apply_bwd(float x)
    {
        return makeTuple(zap(x), BwdCallableForZap(x));
    }
}

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
