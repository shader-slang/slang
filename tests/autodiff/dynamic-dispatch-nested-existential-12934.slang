// Regression test for https://github.com/shader-slang/slang/issues/12934
//
// Registering two `IMaterial` conformances makes `material.setup(...)` a real dynamic
// dispatch; the `StandardMaterial` path reaches the shared `resolve_value -> get_value`, which
// unpacks `IHit` existentials. That gives each `IHit` value a singleton UntaggedUnionType info in
// the type-flow specialization pass, exercising both analyzers that used to reject that shape:
//   - `Surface.get_accessor` downcasts it (`unpacked as TriangleHit` -> ExtractExistentialType);
//   - `get_value` calls an interface method on it (`h.weight()` -> ExtractExistentialWitnessTable
//     + LookupWitnessMethod), with the result used in the return value so the call stays live.
// Must compile to SPIR-V. Both conformances are required: either single conformance keeps the
// material object monomorphic.

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
        // Load-bearing: the `as TriangleHit` downcast produces the ExtractExistentialType over a
        // singleton UntaggedUnionType info that analyzeExtractExistentialType must handle. The
        // identical branches are intentional -- do not collapse this into a plain return.
        if (let triangle = unpacked as TriangleHit)
            return TriangleAccessor();
        return TriangleAccessor();
    }
}

[TreatAsDifferentiable]
float get_value(no_diff Surface surface)
{
    IAccessor accessor = surface.get_accessor();
    // Load-bearing: `h.weight()` is an interface-method call on a singleton-untagged `IHit`
    // value, producing an ExtractExistentialWitnessTable + LookupWitnessMethod that
    // analyzeExtractExistentialWitnessTable must handle. Its result is used in the returned
    // value (flows to `output`), so the front-end cannot drop the call as dead code -- the
    // extraction is genuinely generated for the type-flow pass to resolve.
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
    // Load-bearing downcast (see get_accessor): keep it even though surface.hit is already
    // zero-initialized -- it is required to produce the singleton untagged-union type-flow path.
    if (let triangle = hit as TriangleHit)
        surface.hit = PackedHit();

    IMaterialInstance material = get_material_instance(surface, 0);
    output[0] = material.evaluate(1.0);
}
