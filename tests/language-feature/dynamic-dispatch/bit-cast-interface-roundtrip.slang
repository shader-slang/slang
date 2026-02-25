// bit_cast between interface-typed values and their raw memory representation.
//
// Currently, the compiler lowers interface-typed existentials to a compact
// tagged-union representation when the concrete type set is known, rather than
// the full existential tuple (RTTI + WitnessTableID + AnyValue).  Because the
// tagged-union size depends on the conforming types, the user cannot predict
// the existential's byte-level layout at compile time, and bit_cast fails with
// a size mismatch.
//
// Once issue #9920 is resolved, the compiler should expose a stable existential
// layout so that bit_cast<RawType>(interfaceValue) and
// bit_cast<IFoo>(rawValue) produce correct round-trip results.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"

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

// Raw representation sized for the full existential tuple:
// RTTI [uint2 = 8B] + WitnessTableID [uint2 = 8B] + AnyValue [16B] = 32B = 8 uints
struct RawExistential
{
    uint data[8];
}

float testRoundTrip(int typeId, float val)
{
    IFoo foo = createDynamicObject<IFoo>(typeId, val);
    // CHECK: error 41202
    RawExistential raw = bit_cast<RawExistential>(foo);
    IFoo foo2 = bit_cast<IFoo>(raw);
    return foo2.eval();
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    outputBuffer[0] = testRoundTrip(id, 3.0);
}
