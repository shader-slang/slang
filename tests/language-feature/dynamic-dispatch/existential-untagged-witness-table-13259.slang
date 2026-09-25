// Regression test for https://github.com/shader-slang/slang/issues/13259
//
// A non-autodiff, non-nested-hit companion to the #12934 test; it must compile to SPIR-V. The
// mechanism this exercises is documented at `shade()` below.

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
    // Load-bearing: the dynamically dispatched `mat.setup(...)` (whose result is intentionally
    // unused) together with the direct `si.properties().read()` are what drive the shared
    // `IProperty` value to the singleton untagged-union info in the type-flow pass. Keep both;
    // dropping either makes the object monomorphic and the untagged-union arm is never reached.
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
