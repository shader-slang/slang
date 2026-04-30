// Exercise the fallback `lowerExtractDynamicObject` path in
// slang-ir-lower-dynamic-dispatch-insts.cpp.
//
// When the type-flow specialization pass can see enough context, it
// rewrites `ExtractDynamicObject` directly (the common path tested in
// serialize-dynamic-object.slang). When typeflow can't propagate the
// concrete witness-table set through a function boundary, the inst
// survives into the lowering pass and `lowerExtractDynamicObject`
// reads the existential out of its lowered tuple form. A `[noinline]`
// boundary blocks the propagation and forces the fallback.
//
// This is a compile-only test — the value of the test is regression
// coverage on the fallback's tuple-shape handling, not runtime
// correctness (which is exercised in serialize-dynamic-object.slang).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"
// SPIRV-asm coverage: the fallback's `emitUnpackAnyValue` is rewritten
// by the any-value marshalling pass before SPIRV emission. A direct
// `-target spirv-asm` compile pins that the marshalling pass actually
// eliminates the unpack op for the IR shape this fallback produces;
// the indirect `-vk` coverage in serialize-dynamic-object.slang only
// exercises the typeflow-specialised path, not this fallback path.
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"

[anyValueSize(16)]
interface IFoo
{
    float eval();
}

export struct FooA : IFoo
{
    float x;
    float eval() { return x; }
}

export struct FooB : IFoo
{
    float x;
    float eval() { return x * 2.0; }
}

// The noinline boundary keeps the IFoo argument opaque to the
// type-flow pass when this function is analyzed in isolation, so the
// `serializeDynamicObject` inside survives into the lowering pass.
[noinline]
void doExtract(IFoo foo, out uint typeId, out uint[4] payload)
{
    serializeDynamicObject(foo, typeId, payload);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo foo = createDynamicObject<IFoo>(0, 1.0);
    uint typeId;
    uint[4] payload;
    doExtract(foo, typeId, payload);
    outputBuffer[0] = typeId;
    for (int i = 0; i < 4; ++i)
        outputBuffer[1 + i] = payload[i];

    // CHECK: void computeMain
    // SPIRV: OpEntryPoint
    // SPIRV-NOT: OpUnpackAnyValue
}
