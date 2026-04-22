// `bit_cast` from raw memory into an interface-typed existential is rejected.
// Reconstructing an existential from raw bytes is a compiler-managed
// operation: use `createDynamicObject(typeId, value)` instead.  See
// serialize-dynamic-object.slang for a working round-trip example.
//
// See #10221 / #10222; parent feature tracker is #9920.

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

IFoo testReverse(RawExistential raw)
{
    return bit_cast<IFoo>(raw);
//CHECK:                 ^ cannot bit-cast to existential (non-concrete) type
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    RawExistential raw;
    for (int i = 0; i < 8; i++)
        raw.data[i] = 0;
    outputBuffer[0] = testReverse(raw).eval();
}
