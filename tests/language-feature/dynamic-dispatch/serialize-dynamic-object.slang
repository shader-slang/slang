// Exercise `serializeDynamicObject` (existential -> raw) paired with
// `createDynamicObject` (raw -> existential) to round-trip an
// interface-typed value through a raw uint buffer.  These are the
// supported replacements for `bit_cast` on interface types; see
// bit-cast-interface-roundtrip.slang (forward direction rejection) and
// bit-cast-interface-serialize.slang (reverse direction rejection) for
// the rejected `bit_cast` cases.
//
// Covers #10221 / #10222; parent tracker #9920.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

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

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // FooA with payload 3.0 -> eval returns 3.0.
    IFoo fooA = createDynamicObject<IFoo>(0, 3.0);
    uint typeIdA;
    uint[4] payloadA;
    serializeDynamicObject(fooA, typeIdA, payloadA);

    IFoo restoredA = createDynamicObject<IFoo>(typeIdA, payloadA);
    outputBuffer[0] = restoredA.eval();
    // CHECK: 3.0

    // FooB with payload 4.0 -> eval returns 8.0.
    IFoo fooB = createDynamicObject<IFoo>(1, 4.0);
    uint typeIdB;
    uint[4] payloadB;
    serializeDynamicObject(fooB, typeIdB, payloadB);

    IFoo restoredB = createDynamicObject<IFoo>(typeIdB, payloadB);
    outputBuffer[1] = restoredB.eval();
    // CHECK-NEXT: 8.0

    // The type-ID round-trips exactly.
    outputBuffer[2] = float(typeIdA);
    // CHECK-NEXT: 0.0
    outputBuffer[3] = float(typeIdB);
    // CHECK-NEXT: 1.0
}
