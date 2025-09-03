//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces as constraints on generic interface requirements.

interface IZig
{
    static T doThing<T : IFloat>(T a) : IForwardDifferentiable<__func_as_type(This.doThing<T>)>;
}

struct ZigImpl : IZig
{
    static T doThing<T : IFloat>(T a)
    {
        return a * (T)2;
    }
}

extension<T : IFloat> __func_as_type(ZigImpl::doThing<T>) : IForwardDifferentiable<__func_as_type(ZigImpl::doThing<T>)>
{ }

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IZig izig = ZigImpl();
    outputBuffer[tig] = izig.doThing<float>.fwd_diff(diffPair(3.0)).d; // Expect: 6
}
