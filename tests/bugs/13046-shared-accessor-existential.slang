// Regression test for https://github.com/shader-slang/slang/issues/13046
//
// A shared interface-returning factory (`get_accessor`) is used both during generic material
// setup (early read, via `Material.initialize`) and later through the returned instance (late
// read, via `ConcreteInstance.evaluate`). An earlier specialization phase lowers the factory's
// return type to an `IRTaggedUnionType`; the type-flow specialization pass then took the
// concrete-return fallback in `propagateInterproceduralEdge` and re-wrapped that already-lowered
// union into `UntaggedUnion(TypeSet(TaggedUnion(...)))`, which `analyzeExtractExistentialType`
// rejected with "Unhandled info type". This test verifies the shader compiles instead of aborting.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_6 -entry main

interface IAccessor { float read(); }
struct AccessorA : IAccessor { float read() { return 1; } }
struct AccessorB : IAccessor { float read() { return 2; } }
IAccessor get_accessor(uint id)
{
    if (id == 0) return AccessorA();
    return AccessorB();
}
struct Interaction
{
    uint id;
    IAccessor accessor() { return get_accessor(id); }
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
    float evaluate() { return get_accessor(id).read() + cached; }
}
struct Material : IMaterial
{
    typealias Instance = ConcreteInstance;
    float initialize(Interaction si) { return si.accessor().read(); }
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
