//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces as constraints on generic interface requirements.

interface IFoo
{
    static uint bar(uint a);
}

interface IZig
{
    static T doThing<T : IFloat>(T a) : IFoo;
}

struct ZigImpl : IZig
{
    static T doThing<T : IFloat>(T a)
    {
        return a * (T)2;
    }
}

extension<T : IFloat> __func_as_type(ZigImpl::doThing<T>) : IFoo
{
    static uint bar(uint x)
    {
        return x * x;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IZig izig = ZigImpl();
    outputBuffer[tig] = izig.doThing<float>.bar(4); // Expect: 16
}

/*
// ZigImpl's witness table:
doThing := Generic(T, T:IFloat){ return Func(body); }
(doThing : IFoo) := Generic(T, T:IFloat){ return Specialize(witnessTableDoThingIFoo, T, T:IFloat) }

witnessTableDoThingIFoo := Generic(T, T:IFloat)
{
    return WitnessTable
    {
        bar := Specialize(funcBarExtensionZigImpl_dothing, T, T:IFloat)
    };
}

funcBarExtensionZigImpl_dothing := Generic(T, T:IFloat)
{
    return Func(body); // Lower "bar" in the extension.
}

// --------------------------------

interface IFoo
{
    static uint bar(uint a);
}

interface IZig
{
    static T doThing<T : IFloat>(T a) : IFoo;
}

struct ZigImpl<int N> : IZig
{
    static T doThing<T : IFloat>(T a)
    {
        return a * (T)N;
    }
}

extension<T : IFloat, int N> __func_as_type(ZigImpl<N>::doThing<T>) : IFoo
{
    static uint bar(uint x)
    {
        return x * x * N;
    }
}

// ZigImpl's witness table:
zigImplWitnessTable := Generic(int N)
{
    doThing := Generic(T, T:IFloat){ return Func(body); }
    (doThing : IFoo) := Generic(T, T:IFloat){ return Specialize(witnessTableDoThingIFoo, T, N, T:IFloat) }
}

witnessTableDoThingIFoo := Generic(T, N, T:IFloat)
{
    return WitnessTable<IFoo>
    {
        bar := Specialize(funcBarExtensionZigImpl_dothing, T, N, T:IFloat)
    };
}

funcBarExtensionZigImpl_dothing := Generic(T, N, T:IFloat)
{
    return Func(body); // Lower "bar" in the extension.
}
*/
// -------------------------------

