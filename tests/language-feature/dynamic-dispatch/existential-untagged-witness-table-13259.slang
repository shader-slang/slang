// Regression test for https://github.com/shader-slang/slang/issues/13259
//
// A non-autodiff trigger of the same failure as the #12934 test. `shade()` calls `si.properties()`
// directly, so an earlier `specializeDynamicInsts` run rewrites the declared `IProperty` result type
// of `Surface.properties()` to a `TaggedUnionType`. `Material.setup` is reached only through the
// dispatched `mat.setup(...)`, so its call to `si.properties()` is first analyzed in a later run
// and sees that rewritten type. If `makeInfoForConcreteType` wrapped it as a payload, the `read()`
// call would stop compilation at `Unhandled info type in analyzeExtractExistentialWitnessTable`.
// Must compile to SPIR-V.
//
// `[__unsafeForceInlineEarly]` on `properties(uint)` and the `Surface.properties()` forwarder come
// from the issue's reproducer; the late call sees the rewritten type without either.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute

interface IProperty
{
    float read();
}
struct PropertyA : IProperty
{
    float read() { return 1.0; }
}
struct PropertyB : IProperty
{
    float read() { return 2.0; }
}

[__unsafeForceInlineEarly]
IProperty properties(uint id)
{
    if (id == 0)
        return PropertyA();
    return PropertyB();
}

struct Surface
{
    uint id;
    IProperty properties() { return ::properties(id); }
}

interface IInstance
{
    float evaluate();
}
struct Instance : IInstance
{
    float value;
    float evaluate() { return value; }
}

interface ISampler
{
}
struct Sampler : ISampler
{
}

interface IMaterial
{
    associatedtype Result : IInstance;
    Result setup<S : ISampler>(Surface si, S sampler);
}
struct Material : IMaterial
{
    typedef Instance Result;
    Result setup<S : ISampler>(Surface si, S sampler) { return Instance(si.properties().read()); }
}
struct OtherMaterial : IMaterial
{
    typedef Instance Result;
    Result setup<S : ISampler>(Surface si, S sampler) { return Instance(0.0); }
}
IMaterial material(uint id)
{
    if (id == 0)
        return Material();
    return OtherMaterial();
}

float shade(Surface si, IMaterial mat)
{
    // The dispatched `mat.setup(...)` (its result is intentionally unused) makes `Material.setup`,
    // which holds the late call, reachable; the direct `si.properties().read()` is the earlier call
    // that gets `Surface.properties()` specialized. Without either, no call sees a rewritten type.
    let instance = mat.setup(si, Sampler());
    return si.properties().read();
}

RWStructuredBuffer<float> output;

// CHECK: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    output[tid.x] = shade(Surface(tid.x), material(tid.y));
}
