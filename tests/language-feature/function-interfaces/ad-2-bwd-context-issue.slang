//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0, 0, 0, 0, 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;


interface IFoo
{
    static float zap(uint i) : IBwdDiffApplicable<__func_as_type(This.zap)>;
};

struct FooImpl : IFoo
{
    static float zap(uint i)
    {
        return outputBuffer[i];
    }
};

struct BwdCallableForZap : IBwdCallable<__func_as_type(FooImpl::zap)>
{
    uint idx;
    void operator()(float.Differential dOut)
    {
        outputBuffer[idx + 4] = dOut;
    }
};

extension __func_as_type(FooImpl::zap) : IBwdDiffApplicable<__func_as_type(FooImpl::zap)>
{
    typealias BwdCallable = BwdCallableForZap;
    static Tuple<float, This.BwdCallable> apply_bwd(uint i)
    {
        return makeTuple(FooImpl::zap(i), BwdCallableForZap(i));
    }

    // Synthesized from apply_for_bwd, if present.
    //
    // Alternatively, if apply_for_bwd_callable_only is present and apply_for_bwd is not,
    // then apply_for_bwd is synthesized from apply_for_bwd_callable_only & the original function
    // itself.
    //
    static This.BwdCallable apply_for_bwd_callable_only(uint i)
    {
        return BwdCallableForZap(i);
    }
}

float calc(float x)
{
    float result = 0;
    for (uint i = 0; i < 5; i++)
    {
        result += zap(i);
    }
    return result;
}

struct __synthesized_BwdCallableForCalc : IBwdCallable<__func_as_type(calc)>
{
    float x;
    void operator()(out float.Differential dx, float.Differential dOut)
    {
        dx = 0;
        for (uint i = 0; i < 5; i++)
        {
            dx += zap.apply_for_bwd_callable_only(dOut);
        }
    }
};

Tuple<float, This.BwdCallable> __synthesized_bwd_calc(float x)
{
    BwdCallableForZap bwdCallables[5];
    float result = 0;
    for (uint i = 0; i < 5; i++)
    {
        let resultAndCallable = zap.apply_for_bwd(i);
        result += resultAndCallable._0;
    }

    return makeTuple(result, __synthesized_BwdCallableForCalc(x, bwdCallables));
}


[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IFoo foo = FooImpl();
    var resultTuple = foo.zap.apply_for_fwd(1.f);
    outputBuffer[tig] = resultTuple._0;
    outputBuffer[tig + 1] = resultTuple._1(2.0); // Expect: 4.0
    outputBuffer[tig + 2] = resultTuple._1(1.5); // Expect: 3.0
}
