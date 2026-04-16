//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test that upcasting an existential from a derived interface to a base
// interface compiles and produces correct results (GitHub #10156).

interface IBase
{
    float getValue();
}

interface IDerived : IBase
{
    float transform(float input);
}

interface IGrandchild : IDerived
{
    float extra();
}

struct Impl : IDerived
{
    float v;
    float getValue() { return v; }
    float transform(float input) { return input + v; }
}

struct GrandchildImpl : IGrandchild
{
    float v;
    float getValue() { return v; }
    float transform(float input) { return input + v; }
    float extra() { return v * 2.0; }
}

float useBase(IBase base)
{
    return base.getValue();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Impl impl;
    impl.v = 10.0;
    IDerived derived = impl;

    // Interface-to-interface upcast: IDerived -> IBase
    IBase base = derived;
    outputBuffer[0] = base.getValue();

    // Also test through a function call taking IBase
    outputBuffer[1] = useBase(derived);

    // Multi-level transitive upcast: IGrandchild -> IBase (skipping IDerived)
    GrandchildImpl gcImpl;
    gcImpl.v = 20.0;
    IGrandchild gc = gcImpl;
    IBase transBase = gc;
    outputBuffer[2] = transBase.getValue();

    // Sequential upcast: IGrandchild -> IDerived -> IBase
    IDerived mid = gc;
    IBase seqBase = mid;
    outputBuffer[3] = seqBase.getValue();
}

// CHECK: 10.0
// CHECK: 10.0
// CHECK: 20.0
// CHECK: 20.0
