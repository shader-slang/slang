//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -target hlsl -profile sm_6_5 -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -target metal -entry main -stage raygeneration

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct SharedClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) { input.payload.value += 1; }
}

struct SharedHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SharedClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

// A second logical record in the first schema deliberately reuses the same concrete stage.
struct RepeatedStageHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SharedClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct CustomAttributes
{
    uint value;
}

struct CustomHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = void;
}

struct SharedCustomClosestHit : rt::IClosestHitShader
{
    typealias Context = CustomHitContext;
    void invoke(rt::ClosestHitInput<CustomHitContext> input)
    {
        input.payload.value += input.attributes.value;
    }
}

struct SharedCustomIntersection : rt::IIntersectionShader
{
    typealias Context = CustomHitContext;
    void invoke(rt::IntersectionInput<CustomHitContext> input)
    {
        CustomAttributes attributes = { 2 };
        input.reportHit(1.0, attributes);
    }
}

struct SharedCustomHitGroup : rt::IHitGroup
{
    typealias Context = CustomHitContext;
    typealias ClosestHit = SharedCustomClosestHit;
    typealias AnyHit = rt::NoAnyHit<CustomHitContext>;
    typealias Intersection = SharedCustomIntersection;
}

// These two group types describe different logical function indices, but their concrete stages,
// context, and custom-attribute representation are identical.
struct RepeatedCustomHitGroup : rt::IHitGroup
{
    typealias Context = CustomHitContext;
    typealias ClosestHit = SharedCustomClosestHit;
    typealias AnyHit = rt::NoAnyHit<CustomHitContext>;
    typealias Intersection = SharedCustomIntersection;
}

struct FirstSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<
        SharedHitGroup,
        RepeatedStageHitGroup,
        SharedCustomHitGroup,
        RepeatedCustomHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct SecondSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<SharedHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<FirstSchema> firstProgram;
rt::TraceProgramDescriptor<SecondSchema> secondProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<FirstSchema> firstTracer;
    firstTracer.trace(desc, scene, firstProgram, payload);
    rt::RayTracer<SecondSchema> secondTracer;
    secondTracer.trace(desc, scene, secondProgram, payload);
}

// Multiple groups in FirstSchema and another group in SecondSchema reference the same semantic
// triangle stage. FirstSchema also references one custom closest-hit stage from two groups. The
// D3D/Vulkan path therefore emits each concrete native entry point only once.
// HLSL-COUNT-2: [shader("closesthit")]void

// The generated Metal ray data stores committed custom attributes by their concrete type. Both
// custom groups therefore share this one field instead of allocating one field per group.
// METAL-LABEL: struct FirstSchema_rayData_0
// METAL: CustomAttributes_0 {{.*}}attributes_{{[0-9]+}};
// METAL-NOT: CustomAttributes_0
// METAL: };

// Metal keeps the schema-specific ray-data type in the physical signature, but a logical group
// index is not part of a stage adapter's identity. FirstSchema consequently emits two adapters:
// one for SharedClosestHit and one for SharedCustomClosestHit, even though four groups reference
// them. SecondSchema requires its own adapter because it has a different ray-data ABI type.
// METAL-COUNT-2: {{\[\[visible\]\]}} void {{.*}}(FirstSchema_rayData_0 thread*
// METAL-COUNT-1: {{\[\[visible\]\]}} void {{.*}}(SecondSchema_rayData_0 thread*
