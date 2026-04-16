// Manual serialization of interface-typed values into a raw uint buffer using
// bit_cast, then deserialization back to interface type.
//
// Fixes: #10221, #10222 (parent: #9920)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=storageBuf
RWStructuredBuffer<uint> storageBuf;

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

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
    RawExistential raw = bit_cast<RawExistential>(foo);
    for (int i = 0; i < kRawUints; i++)
        buf[baseSlot + i] = raw.data[i];
}

IFoo deserialize(RWStructuredBuffer<uint> buf, int baseSlot)
{
    RawExistential raw;
    for (int i = 0; i < kRawUints; i++)
        raw.data[i] = buf[baseSlot + i];
    return bit_cast<IFoo>(raw);
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFoo fooA = createDynamicObject<IFoo>(0, 3.0);
    serialize(fooA, storageBuf, 0);
    IFoo restored = deserialize(storageBuf, 0);
    outputBuffer[0] = restored.eval();
    // CHECK: 3.0
}
