// Regression test for https://github.com/shader-slang/slang/issues/12934
//
// Registering two `IMaterial` conformances makes `material.setup(...)` a real dynamic dispatch;
// the `StandardMaterial` path reaches the shared `resolve_value -> get_value ->
// Surface.get_accessor`, which calls `PackedHit.unpack()` and downcasts its `IHit` result with
// `as TriangleHit`. An earlier specialization phase lowers that `IHit` value to an
// `IRTaggedUnionType`; the type-flow specialization pass then re-wrapped the already-lowered union
// into `UntaggedUnion(TypeSet(TaggedUnion(...)))`, which `analyzeExtractExistentialType` rejected
// with "Unhandled info type" -- the same producer double-wrap as #13046. Both conformances are
// required: either single conformance keeps the material object monomorphic and never triggers the
// shared refinement. Must compile to SPIR-V.
//
// Autodiff is the context #12934 was reported in, not a requirement of the crash: the double-wrap
// mechanism is autodiff-independent (the `tests/bugs/13046-*` siblings reproduce the same producer
// double-wrap with no differentiation). This test keeps #12934's original differentiable form
// (hence `tests/autodiff/`) as the faithful reduction of the reported repro.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute -conformance "InvalidMaterial:IMaterial=0" -conformance "StandardMaterial:IMaterial=1"

interface IHit
{ }

RWStructuredBuffer<float> output;

struct TriangleHit : IHit
{ }

struct PackedHit
{
    IHit unpack()
    {
        return TriangleHit();
    }
}

interface IAccessor
{
    float get();
}

struct TriangleAccessor : IAccessor
{
    float get() { return 2.0; }
}

struct Surface : IDifferentiable
{
    float uv;
    no_diff PackedHit hit;

    IAccessor get_accessor()
    {
        IHit unpacked = hit.unpack();
        // Load-bearing: the `as TriangleHit` downcast over the unpacked `IHit` produces the
        // ExtractExistentialType whose operand carries that value's already-lowered tagged-union
        // info. The identical branches are intentional -- do not "simplify" this into a plain
        // `return TriangleAccessor()`.
        if (let triangle = unpacked as TriangleHit)
            return TriangleAccessor();
        return TriangleAccessor();
    }
}

[TreatAsDifferentiable]
float get_value(no_diff Surface surface)
{
    IAccessor accessor = surface.get_accessor();
    return accessor.get();
}

[Differentiable]
float resolve_value(Surface surface)
{
    return get_value(surface) + surface.uv;
}

interface IMaterialInstance
{
    float evaluate(float value);
}

struct StandardMaterialInstance : IMaterialInstance
{
    float scale;

    float evaluate(float value) { return scale * value; }
}

interface IMaterial
{
    associatedtype MaterialInstance : IMaterialInstance;

    MaterialInstance setup<S>(Surface surface, S sampler);
}

struct StandardMaterial : IMaterial
{
    typedef StandardMaterialInstance MaterialInstance;
    uint data;

    MaterialInstance setup<S>(Surface surface, S sampler)
    {
        MaterialInstance result = {};
        result.scale = resolve_value(surface) * float(data);
        return result;
    }
}

struct InvalidMaterial : IMaterial
{
    typedef StandardMaterialInstance MaterialInstance;
    uint data;

    MaterialInstance setup<S>(Surface surface, S sampler)
    {
        return {};
    }
}

IMaterial get_material()
{
    uint data = 2;
    return createDynamicObject<IMaterial, uint>(1, data);
}

[Differentiable]
IMaterialInstance get_material_instance<S>(no_diff Surface surface, S sampler)
{
    IMaterial material = get_material();
    return material.setup(surface, sampler);
}

// CHECK: OpEntryPoint
[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main()
{
    PackedHit packed_hit = {};
    IHit hit = packed_hit.unpack();

    Surface surface = {};
    // Load-bearing downcast (see get_accessor): keep it even though surface.hit is already
    // zero-initialized -- it is required to route the shared accessor through the type-flow
    // refinement path that reproduced the crash.
    if (let triangle = hit as TriangleHit)
        surface.hit = PackedHit();

    IMaterialInstance material = get_material_instance(surface, 0);
    output[0] = material.evaluate(1.0);
}
