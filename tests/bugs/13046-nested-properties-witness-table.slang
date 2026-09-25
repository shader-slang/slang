// Regression test for https://github.com/shader-slang/slang/issues/13046 — reporter's standalone
// reducer, covering both consumers of the producer double-wrap.
//
// This combines #12934's nested-interface pattern (an `IHit` whose `properties()` returns another
// interface `IProperty`) with #13046's setup/evaluate split (`IMaterial.setup` produces a
// `ConcreteInstance` whose `evaluate()` re-reads the shared property factory). Two `IMaterial`
// conformances make `material.setup(...)` a real dynamic dispatch; the shared
// `get_properties -> get_hit(id).properties()` factory is refined once, and an earlier
// specialization phase lowers its `IProperty`/`IHit` result to an `IRTaggedUnionType`. The
// type-flow specialization pass then re-wrapped that already-lowered union into
// `UntaggedUnion(TypeSet(TaggedUnion(...)))`.
//
// The two directives below pin the two consumers that reject that malformed shape:
//   - default: aborts in `analyzeExtractExistentialWitnessTable` ("Unhandled info type").
//   - INLINE_HIT_FACTORY=1: inlining `get_hit` moves the refinement so the malformed shape instead
//     reaches `analyzeExtractExistentialType`, the consumer of the original #13046 top-level repro.
// We fix this at the producer: `makeInfoForConcreteType` returns the already-lowered union as-is
// rather than re-lifting it, so the double-wrap never forms upstream of either consumer.
//
// Load-bearing shape (do not "simplify"): both `IMaterial` conformances are required (a monomorphic
// material never triggers the shared refinement); the nested `IHit.properties() -> IProperty` hop is
// what routes an already-lowered union back through the concrete-return fallback; and the late
// re-read in `ConcreteInstance.evaluate()` (LATE_READ) is what forces the shared factory to be
// refined at the setup site. `-DDIRECT_PROPERTIES=1` (property factory stops calling `get_hit`) and
// `-DLATE_READ=0` (no re-read) both compile even pre-fix — they remove the shared refinement — so
// they are documented as negative controls, not tested directives.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile sm_6_7 -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile sm_6_7 -entry main -stage compute -DINLINE_HIT_FACTORY=1

#ifndef INLINE_HIT_FACTORY
#define INLINE_HIT_FACTORY 0
#endif
#ifndef DIRECT_PROPERTIES
#define DIRECT_PROPERTIES 0
#endif
#ifndef LATE_READ
#define LATE_READ 1
#endif
interface IProperty { float read(); }
struct PropertyA : IProperty { float read() { return 1.0; } }
struct PropertyB : IProperty { float read() { return 2.0; } }
interface IHit { IProperty properties(); }
struct HitA : IHit { IProperty properties() { return PropertyA(); } }
struct HitB : IHit { IProperty properties() { return PropertyB(); } }
#if INLINE_HIT_FACTORY
[__unsafeForceInlineEarly]
#endif
IHit get_hit(uint id)
{
    if (id == 0) return HitA();
    return HitB();
}
[__unsafeForceInlineEarly]
IProperty get_properties(uint id)
{
#if DIRECT_PROPERTIES
    if (id == 0) return PropertyA();
    return PropertyB();
#else
    return get_hit(id).properties();
#endif
}
struct Surface
{
    uint id;
    IProperty properties() { return get_properties(id); }
}
interface IInstance { float evaluate(); }
interface ISampler { }
struct Sampler : ISampler { }
interface IMaterial
{
    associatedtype Instance : IInstance;
    Instance setup<S : ISampler>(Surface si, S sampler);
}
struct ConcreteInstance : IInstance
{
    uint id;
    float cached;
    float evaluate()
    {
#if LATE_READ
        return get_properties(id).read() + cached;
#else
        return cached;
#endif
    }
}
struct Material : IMaterial
{
    typealias Instance = ConcreteInstance;
    float initialize(Surface si) { return si.properties().read(); }
    Instance setup<S : ISampler>(Surface si, S sampler) { return Instance(si.id, initialize(si)); }
}
struct OtherMaterial : IMaterial
{
    typealias Instance = ConcreteInstance;
    Instance setup<S : ISampler>(Surface si, S sampler) { return Instance(si.id, 0.0); }
}
IMaterial get_material(uint id)
{
    if (id == 0) return Material();
    return OtherMaterial();
}
float shade(Surface si, IMaterial material)
{
    return material.setup(si, Sampler()).evaluate();
}
RWStructuredBuffer<float> output;

// CHECK: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[tid.x] = shade(Surface(tid.x), get_material(tid.y));
}
