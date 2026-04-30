// `bit_cast` TO a type that transitively contains an interface type
// is rejected at IR layer with E41204 ("cannot bit-cast to existential
// (non-concrete) type").  The semantic-check E41205 fires for the
// operand side (see bit-cast-interface-wrapped.slang); the result side
// is caught by the `isConcreteType` guard in type-flow specialization.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0"

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

struct Wrapper
{
    IFoo foo;
    float extra;
}

struct RawData { uint data[12]; }

Wrapper testReverseStruct(RawData raw)
{
    return bit_cast<Wrapper>(raw);
//CHECK:                    ^ cannot bit-cast to existential (non-concrete) type
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    RawData r;
    for (int i = 0; i < 12; i++)
        r.data[i] = 0;
    outputBuffer[0] = testReverseStruct(r).extra;
}
