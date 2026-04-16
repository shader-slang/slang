// bit_cast between interface-typed values and their raw memory representation.
//
// Validates that bit_cast<RawType>(interfaceValue) and
// bit_cast<IFoo>(rawValue) produce correct round-trip results, preserving
// the full existential representation (RTTI + WitnessTableID + AnyValue).
//
// Fixes: #10221, #10222 (parent: #9920)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

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
    return foo2.eval();
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FooA.eval() returns x directly: 3.0
    outputBuffer[0] = testRoundTrip(0, 3.0);
    // CHECK: 3.0

    // FooB.eval() returns x * 2.0: 6.0
    outputBuffer[1] = testRoundTrip(1, 3.0);
    // CHECK: 6.0
}
