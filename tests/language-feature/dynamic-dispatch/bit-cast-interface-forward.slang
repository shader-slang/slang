// `bit_cast` from an interface-typed existential into a raw struct is
// rejected.  The existential memory layout (witness tables, any-value
// payload) is a compiler-managed detail that must not leak through
// `bit_cast`.  Use `serializeDynamicObject` for the existential -> raw
// direction; see serialize-dynamic-object.slang.
//
// See #10221 / #10222 for context; parent feature tracker is #9920.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

struct FooA : IFoo
{
    float x;
    float eval() { return x; }
}

struct FooB : IFoo
{
    float x;
    float eval() { return x * 2.0; }
}

struct RawExistential
{
    uint data[8];
}

float testForward(int typeId, float val)
{
    IFoo foo = createDynamicObject<IFoo>(typeId, val);
    RawExistential raw = bit_cast<RawExistential>(foo);
//CHECK:                                          ^^^ bit_cast involving interface type is not supported
//CHECK:                                          ^^^ 'IFoo' involves an interface type
    return float(raw.data[0]);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    outputBuffer[0] = testForward(id, 3.0);
}
