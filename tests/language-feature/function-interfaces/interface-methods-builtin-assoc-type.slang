//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces as constraints on interface methods.

interface IZig : IDifferentiable
{
    static float doThing(This a) : IForwardDifferentiable<__func_as_type(This.doThing)>;
}

struct ZigImpl : IZig
{
    float k;
    static float doThing(ZigImpl a)
    {
        return a.k * 2;
    }
}

extension __func_as_type(ZigImpl::doThing) : IForwardDifferentiable<__func_as_type(ZigImpl::doThing)>
{ /* built-in synthesis */ }

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IZig izig = ZigImpl();
    outputBuffer[tig] = izig.doThing.fwd_diff(diffPair(4.0, 3.0)).d; // Expect: 8
}
