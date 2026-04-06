// bit_cast between interface-typed values and their raw memory representation.
//
// Currently, the compiler lowers interface-typed existentials to a compact
// tagged-union representation when the concrete type set is known, rather than
// the full existential tuple (RTTI + WitnessTableID + AnyValue).  Because the
// tagged-union size depends on the conforming types, bit_cast round-tripping
// through an interface-typed existential is not currently supported.
//
// Two sub-issues block this feature:
//   #10221 — bit_cast extracts concrete payload instead of full existential
//   #10222 — specialization lowers existential to compact tagged-union
//
// Once resolved, bit_cast<RawType>(interfaceValue) and
// bit_cast<IFoo>(rawValue) should produce correct round-trip results.

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

// Raw representation sized for the full existential tuple:
// RTTI [uint2 = 8B] + WitnessTableID [uint2 = 8B] + AnyValue [16B] = 32B = 8 uints
static const int kRawUints = 8;

struct RawExistential
{
    uint data[kRawUints];
}

float testRoundTrip(int typeId, float val)
{
    IFoo foo = createDynamicObject<IFoo>(typeId, val);
    RawExistential raw = bit_cast<RawExistential>(foo);
    IFoo foo2 = bit_cast<IFoo>(raw);
//CHECK:                      ^ cannot bit-cast to existential (non-concrete) type
    return foo2.eval();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    outputBuffer[0] = testRoundTrip(id, 3.0);
}
