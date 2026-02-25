// Manual serialization of interface-typed values into a raw uint buffer using
// bit_cast, then deserialization back to interface type.
//
// Blocked on the same two sub-issues as bit-cast-interface-roundtrip:
//   #10221 — bit_cast extracts concrete payload instead of full existential
//   #10222 — specialization lowers existential to compact tagged-union
// See parent issue #9920.

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

static const int kRawUints = 8;

struct RawExistential
{
    uint data[kRawUints];
}

void serialize(IFoo foo, RWStructuredBuffer<uint> buf, int baseSlot)
{
    // CHECK: error 41202
    RawExistential raw = bit_cast<RawExistential>(foo);
    for (int i = 0; i < kRawUints; i++)
        buf[baseSlot + i] = raw.data[i];
}

IFoo deserialize(RWStructuredBuffer<uint> buf, int baseSlot)
{
    RawExistential raw;
    for (int i = 0; i < kRawUints; i++)
        raw.data[i] = buf[baseSlot + i];
    // This cast would also fail, but the compiler bails out after the first
    // error in serialize() so no second diagnostic is emitted today.  A partial
    // fix to #10221 or #10222 could change that; add a CHECK here when it does.
    return bit_cast<IFoo>(raw);
}

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<uint> storageBuf;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo fooA = createDynamicObject<IFoo>(id, 3.0);
    serialize(fooA, storageBuf, 0);
    IFoo restored = deserialize(storageBuf, 0);
    outputBuffer[0] = restored.eval();
}
