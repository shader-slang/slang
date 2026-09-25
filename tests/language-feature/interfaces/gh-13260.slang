// Regression test for https://github.com/shader-slang/slang/issues/13260
//
// `IHit` has a default *static* method `value(Packed)` whose name collides with the instance
// requirement `value()`. `Hit` overrides only the instance `value()`; the static `value(Packed)`
// must inherit the interface default. The witness synthesizer previously bound the static
// requirement's `Packed` parameter as the instance method's implicit `this`, producing an invalid
// forwarder: HLSL/CUDA emitted a call passing a `Packed` to `Hit::value(Hit this)`, and SPIR-V
// hit an internal assert at slang-ir.cpp:5822. The synthesized witness is only exercised through
// dynamic dispatch, hence `createDynamicObject` here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry compute_main -stage compute -profile sm_6_7 -conformance Hit:IHit=0

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
    static This unpack(Packed p) { return {p.instance, p.uv}; }
    override float value() { return float(instance) + uv.x + uv.y; }
}

StructuredBuffer<Packed> input;
RWStructuredBuffer<float> output;

// The static `value(Packed)` requirement, reached through a dynamically created object, must
// resolve to the inherited default (reconstruct a Hit via unpack, then call its instance value()).
// Previously this asserted internally; we only need it to compile to valid SPIR-V now.
//CHECK: OpEntryPoint
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    let type = createDynamicObject<IHit>(0u, 0u);
    output[tid.x] = type.value(input[tid.x]);
}
