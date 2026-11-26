//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

// Test an `Optional` of an interface type whose implementation holds another interface-typed
// field. The storage for the outer interface value holds the storage for the inner one, so the
// inner one has to be sized first.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInner
{
    float get();
}

struct InnerA : IInner
{
    float a;
    float get() { return a; }
}

struct InnerB : IInner
{
    float4 b;
    float get() { return b.x; }
}

interface IOuter
{
    float eval();
}

struct OuterA : IOuter
{
    IInner inner;
    float eval() { return inner.get(); }
}

struct OuterB : IOuter
{
    float v;
    float eval() { return v; }
}

IInner makeInner(int id)
{
    if (id == 0)
    {
        InnerA a;
        a.a = 1.0;
        return a;
    }
    InnerB b;
    b.b = float4(2.0);
    return b;
}

IOuter makeOuter(int id)
{
    if (id < 2)
    {
        OuterA a;
        a.inner = makeInner(id);
        return a;
    }
    OuterB b;
    b.v = 3.0;
    return b;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    for (int id = 0; id < 4; id++)
    {
        Optional<IOuter> outer = none;
        if (id < 3)
            outer = makeOuter(id);

        float result = -1.0;
        if (let value = outer)
            result = value.eval();
        outputBuffer[id] = result;
    }

    // CHECK: 1.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 3.0
    // CHECK-NEXT: -1.0
}
