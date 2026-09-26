// Regression test for https://github.com/shader-slang/slang/issues/13260
//
// `IHit` has a default *static* method `value(Packed)` whose name collides with the instance
// requirement `value()`. `Hit` overrides only the instance `value()`; the static `value(Packed)`
// must inherit the interface default, which reconstructs a `Hit` via `unpack` and calls its
// instance `value()`. The witness synthesizer previously bound the static requirement's `Packed`
// parameter as the instance method's implicit `this`, so `Hit`'s witness for `value(Packed)`
// became a forwarder that called `Hit::value(Hit this)` with a `Packed`: HLSL/CUDA emitted that
// invalid call, and SPIR-V hit an internal assert in `emitFieldAddress` (looking up a `Hit` field
// on a `Packed` value). The wrong witness is chosen at conformance-check time, so both a generic
// static call `T.value(p)` and a dynamic `createDynamicObject<IHit>().value(p)` use it; both are
// exercised below.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute -conformance Hit:IHit=0

struct Packed
{
    uint instance;
    float2 uv;
}

interface IHit
{
    static This unpack(Packed p);
    float value() { return 0.0; }
    static float value(Packed p) { return This.unpack(p).value(); }
}

struct Hit : IHit
{
    uint instance;
    float2 uv;
    static This unpack(Packed p) { return {p.instance + 1, p.uv}; }
    override float value() { return float(instance) + uv.x + uv.y; }
}

float viaGeneric<T : IHit>(Packed p)
{
    return T.value(p);
}

//TEST_INPUT: type_conformance Hit:IHit = 0
//TEST_INPUT: set output = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> output;

//SPV: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Packed p = {1, float2(2.0, 3.0)};

    // Both paths must select the inherited default, which reconstructs Hit{2, (2,3)} via unpack
    // (which bumps `instance`) and calls its instance value(): float(2) + 2.0 + 3.0 = 7.0. Reading
    // the Packed argument directly as a Hit would give 6.0 instead.
    output[0] = viaGeneric<Hit>(p); // CHECK: 7.0
    output[1] = createDynamicObject<IHit>(0u, 0u).value(p); // CHECK: 7.0
}
