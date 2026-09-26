// Regression test for https://github.com/shader-slang/slang/issues/12934
//
// `compute_main` calls `PackedHit.unpack()` directly, so an earlier `specializeDynamicInsts` run
// rewrites its declared `IHit` result type to a `TaggedUnionType`. Registering two `IMaterial`
// conformances makes `material.setup(...)` a real dynamic dispatch, so `Surface.get_accessor` and
// `get_value`, reached only through `StandardMaterial.setup`, are first analyzed in a later run, and
// their calls to `unpack()` see that rewritten type. If `makeInfoForConcreteType` wrapped it as a
// payload, the resulting `UntaggedUnionType` info would stop compilation with an internal error in
//   - `analyzeExtractExistentialType`, for the downcast in `get_accessor`, and
//   - `analyzeExtractExistentialWitnessTable`, for the `h.weight()` call in `get_value`.
// Must compile to SPIR-V. With either single conformance the material object is monomorphic and no
// call sees a rewritten type.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute -conformance "InvalidMaterial:IMaterial=0" -conformance "StandardMaterial:IMaterial=1"

interface IHit
{
    float weight();
}

RWStructuredBuffer<float> output;

struct TriangleHit : IHit
{
    float weight() { return 3.0; }
}

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
        // The downcast produces an `ExtractExistentialType` on the value `unpack()` returns, which
        // keeps this late call site live; the identical branches are intentional.
        if (let triangle = unpacked as TriangleHit)
            return TriangleAccessor();
        return TriangleAccessor();
    }
}

[TreatAsDifferentiable]
float get_value(no_diff Surface surface)
{
    IAccessor accessor = surface.get_accessor();
    // `h.weight()` produces an `ExtractExistentialWitnessTable` and a `LookupWitnessMethod` on the
    // value `unpack()` returns. Its result feeds the returned value, so the call is not dead code.
    IHit h = surface.hit.unpack();
    return accessor.get() + h.weight();
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
    // The downcast keeps the direct `packed_hit.unpack()` call live, so the earlier run specializes
    // `unpack()` and rewrites its result type; without it, no call sees a rewritten type.
    if (let triangle = hit as TriangleHit)
        surface.hit = PackedHit();

    IMaterialInstance material = get_material_instance(surface, 0);
    output[0] = material.evaluate(1.0);
}
