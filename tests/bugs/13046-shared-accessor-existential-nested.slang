// Regression test for https://github.com/shader-slang/slang/issues/13046 — nested variant.
//
// Same shared-accessor early/late-read shape as 13046-shared-accessor-existential.slang, but the
// factory returns the interface wrapped in a structural type (`IAccessor[1]`). An earlier phase
// lowers that return to `IAccessor[1]` → `TaggedUnion[1]`, so the type-flow specialization pass
// decomposes the array in `makeInfoForConcreteType` and recurses on the element. This exercises
// the refined-info-nested-in-a-structural-type path: the element is an already-lowered tagged
// union that must be reused as-is rather than re-wrapped. Verifies the shader compiles.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_6 -entry main

interface IAccessor { float read(); }
struct AccessorA : IAccessor { float read() { return 1; } }
struct AccessorB : IAccessor { float read() { return 2; } }
IAccessor[1] get_accessor(uint id)
{
    IAccessor r[1];
    if (id == 0) { r[0] = AccessorA(); } else { r[0] = AccessorB(); }
    return r;
}
struct Interaction
{
    uint id;
    IAccessor[1] accessor() { return get_accessor(id); }
}
interface IInstance { float evaluate(); }
interface ISampler { }
struct Sampler : ISampler { }
interface IMaterial
{
    associatedtype Instance : IInstance;
    Instance setup<S : ISampler>(Interaction si, S sampler);
}
struct ConcreteInstance : IInstance
{
    uint id;
    float cached;
    float evaluate() { return get_accessor(id)[0].read() + cached; }
}
struct Material : IMaterial
{
    typealias Instance = ConcreteInstance;
    float initialize(Interaction si) { return si.accessor()[0].read(); }
    Instance setup<S : ISampler>(Interaction si, S sampler)
    {
        return Instance(si.id, initialize(si));
    }
}
struct OtherMaterial : IMaterial
{
    typealias Instance = ConcreteInstance;
    Instance setup<S : ISampler>(Interaction si, S sampler) { return Instance(si.id, 0); }
}
// Load-bearing: `get_material` returns one of two `IMaterial` implementations so
// `material.setup(...)` is a real dynamic dispatch. Do not simplify to a single material -- a
// monomorphic material never triggers the shared refinement that reproduces the crash.
IMaterial get_material(uint id)
{
    if (id == 0) return Material();
    return OtherMaterial();
}
float shade(Interaction si, IMaterial material)
{
    return material.setup(si, Sampler()).evaluate();
}
RWStructuredBuffer<float> output;

// CHECK: void main
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = shade(Interaction(tid.x), get_material(tid.y));
}
