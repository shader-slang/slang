// Companion to serialize-dynamic-object.slang covering non-uint
// storage types for `serializeDynamicObject` / `createDynamicObject`.
//
// The builtin accepts any storage type `U` whose size matches the
// interface's any-value payload. The existing tests in this PR all
// use `uint[N]`; this file pins that the reinterpret / unpack
// lowering paths also work for storage types with different element
// layouts (here: `float4`, a 16-byte vector). A regression in the
// reinterpret/unpack code that only happens for non-uint storage
// would be invisible to the existing tests.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

struct FooA : IFoo
{
    float a, b, c, d;
    float eval() { return a + b + c + d; }
}

struct FooB : IFoo
{
    float a, b, c, d;
    float eval() { return a * b * c * d; }
}

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // FooA payload (1, 2, 3, 4) -> eval returns 10.
    IFoo fooA = createDynamicObject<IFoo>(0, float4(1, 2, 3, 4));
    uint typeIdA;
    float4 payloadA;
    serializeDynamicObject(fooA, typeIdA, payloadA);

    IFoo restoredA = createDynamicObject<IFoo>(typeIdA, payloadA);
    outputBuffer[0] = restoredA.eval();
    // CHECK: 10.0

    // FooB payload (1, 2, 3, 4) -> eval returns 24.
    IFoo fooB = createDynamicObject<IFoo>(1, float4(1, 2, 3, 4));
    uint typeIdB;
    float4 payloadB;
    serializeDynamicObject(fooB, typeIdB, payloadB);

    IFoo restoredB = createDynamicObject<IFoo>(typeIdB, payloadB);
    outputBuffer[1] = restoredB.eval();
    // CHECK-NEXT: 24.0

    // Type-IDs round-trip through the float4 payload path too.
    outputBuffer[2] = float(typeIdA);
    // CHECK-NEXT: 0.0
    outputBuffer[3] = float(typeIdB);
    // CHECK-NEXT: 1.0
}
