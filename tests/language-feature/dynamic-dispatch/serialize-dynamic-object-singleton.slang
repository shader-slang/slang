// Singleton variant of serialize-dynamic-object.slang.
//
// When only ONE conformance is registered, the type-flow pass takes
// the `typeSet->isSingleton()` branch in `specializeExtractDynamicObject`
// and uses `emitReinterpret` instead of `emitUnpackAnyValue`. The
// multi-conformance test (serialize-dynamic-object.slang) doesn't
// cover that branch, so this file pins the singleton path.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type

// In the singleton case the typeflow pass collapses the packed type
// to the concrete struct, so the user's storage must match the
// struct's size (one float = 4 bytes), not the interface's full
// anyValueSize.  Using uint[1] aligns with FooOnly's layout.
[anyValueSize(16)]
interface IFoo
{
    float eval();
}

struct FooOnly : IFoo
{
    float x;
    float eval() { return x + 1.0; }
}

//TEST_INPUT: type_conformance FooOnly:IFoo = 0

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo foo = createDynamicObject<IFoo>(0, 7.0);
    uint typeId;
    uint[1] payload;
    serializeDynamicObject(foo, typeId, payload);

    IFoo restored = createDynamicObject<IFoo>(typeId, payload);
    outputBuffer[0] = restored.eval();
    // CHECK: 8.0

    outputBuffer[1] = float(typeId);
    // CHECK-NEXT: 0.0
}
