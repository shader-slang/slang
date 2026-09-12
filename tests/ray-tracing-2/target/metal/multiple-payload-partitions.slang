//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

// These deliberately different payload shapes expose accidental sharing in generated Metal
// signatures. In particular, accessing `visibility` through a RadiancePayload carrier cannot
// accidentally compile as an equivalent same-sized integer field access.
struct ShadowPayload
{
    float visibility;
}

struct RadiancePayload
{
    uint radiance;
    uint bounce;
}

// These deliberately different record sizes verify that a shared records buffer has one physical
// hit and miss stride. Both payload partitions must use the schema-wide maximum (48 bytes after
// adding the 16-byte record header), even though RadianceRecord alone would need only 32 bytes.
struct ShadowRecord
{
    float4 value;
    float tail;
}

struct RadianceRecord
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ShadowHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ShadowPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ShadowRecord;
}

struct RadianceHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = RadiancePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = RadianceRecord;
}

struct ShadowMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ShadowPayload;
    typealias Record = ShadowRecord;
}

struct RadianceMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = RadiancePayload;
    typealias Record = RadianceRecord;
}

struct ShadowAnyHit : rt::IAnyHitShader
{
    typealias Context = ShadowHitContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        input.payload.visibility = input.triangle.frontFacing ? 1.0 : 0.0;
    }
}

struct RadianceAnyHit : rt::IAnyHitShader
{
    typealias Context = RadianceHitContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        input.payload.radiance++;
    }
}

struct ShadowClosestHit : rt::IClosestHitShader
{
    typealias Context = ShadowHitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.visibility += input.distance;
    }
}

struct RadianceClosestHit : rt::IClosestHitShader
{
    typealias Context = RadianceHitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.radiance += input.payload.bounce;
    }
}

struct ShadowMiss : rt::IMissShader
{
    typealias Context = ShadowMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.visibility = -1.0;
    }
}

struct RadianceMiss : rt::IMissShader
{
    typealias Context = RadianceMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.radiance = 100;
        input.payload.bounce = 2;
    }
}

struct ShadowHitGroup : rt::IHitGroup
{
    typealias Context = ShadowHitContext;
    typealias ClosestHit = ShadowClosestHit;
    typealias AnyHit = ShadowAnyHit;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct RadianceHitGroup : rt::IHitGroup
{
    typealias Context = RadianceHitContext;
    typealias ClosestHit = RadianceClosestHit;
    typealias AnyHit = RadianceAnyHit;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;

    // Payload partitions follow their first appearance in HitGroups. MissShaders deliberately use
    // the opposite order to prove that discovering an existing payload does not reorder it.
    typealias HitGroups = rt::HitGroupList<ShadowHitGroup, RadianceHitGroup>;
    typealias MissShaders = rt::MissShaderList<RadianceMiss, ShadowMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    rt::RayTracer<Schema> tracer;

    RadiancePayload radiance = {};
    tracer.trace(desc, scene, program, radiance);

    ShadowPayload shadow = {};
    tracer.trace(desc, scene, program, shadow);

    output[0] = radiance.radiance + radiance.bounce + uint(shadow.visibility);
}

// ShadowPayload is payload partition zero because ShadowHitGroup appears first. Its candidate
// adapter additionally requires `triangle_data`, while the independent radiance partition does
// not; this checks that IFT requirements are partitioned along with visible-function signatures.
// METAL: struct Schema_payload0_rayData_0
// METAL: ShadowPayload_0 payload_{{[0-9]+}};
// METAL: struct Schema_payload1_rayData_0
// METAL: RadiancePayload_0 payload_{{[0-9]+}};
// METAL: struct rt_TraceProgramDescriptorResources_default_0
// METAL: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data> intersectionFunctions0_0;
// METAL: visible_function_table<void(Schema_payload0_rayData_0 thread*)> missFunctions0_0;
// METAL: visible_function_table<void(Schema_payload0_rayData_0 thread*, float)> closestHitFunctions0_0;
// METAL: intersection_function_table<metal::raytracing::instancing> intersectionFunctions1_0;
// METAL: visible_function_table<void(Schema_payload1_rayData_0 thread*)> missFunctions1_0;
// METAL: visible_function_table<void(Schema_payload1_rayData_0 thread*)> closestHitFunctions1_0;
// METAL: uint32_t device* callableFunctions_0;
// METAL: uint32_t device* records_0;

// Each trace statically selects its payload partition instead of dynamically selecting a carrier.
// METAL: program_resources_{{[0-9]+}}->intersectionFunctions1_0
// METAL: program_resources_{{[0-9]+}}->missFunctions1_0
// METAL: program_resources_{{[0-9]+}}->closestHitFunctions1_0
// METAL: thread Schema_payload1_rayData_0
// METAL: _slang_miss_record = {{.*}} * int(48)
// METAL: _slang_hit_record = {{.*}} * int(48)
// METAL: program_resources_{{[0-9]+}}->intersectionFunctions0_0
// METAL: program_resources_{{[0-9]+}}->missFunctions0_0
// METAL: program_resources_{{[0-9]+}}->closestHitFunctions0_0
// METAL: thread Schema_payload0_rayData_0
// METAL: _slang_miss_record = {{.*}} * int(48)
// METAL: _slang_hit_record = {{.*}} * int(48)

// The records buffer is schema-wide, so the two payload partitions must resolve the same physical
// record index with the same section strides. A per-payload 32/48 split would address overlapping
// records and fail these checks.

// Candidate and visible-function adapters may only exchange their own payload's ray-data type.
// METAL-LABEL: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_payload0_rayData_0 thread*
// METAL: visibility_0
// METAL-NOT: radiance_0
// METAL-LABEL: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_payload0_rayData_0 thread*
// METAL: visibility_0
// METAL-NOT: radiance_0
// METAL-LABEL: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::triangle_data)]]
// METAL-SAME: ray_data Schema_payload0_rayData_0&
// METAL: visibility_0
// METAL-NOT: radiance_0
// METAL-LABEL: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_payload1_rayData_0 thread*
// METAL: radiance_0
// METAL-NOT: visibility_0
// METAL-LABEL: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_payload1_rayData_0 thread*
// METAL: radiance_0
// METAL-NOT: visibility_0
// METAL-LABEL: {{\[\[}}intersection(triangle, metal::raytracing::instancing)]]
// METAL-SAME: ray_data Schema_payload1_rayData_0&
// METAL: radiance_0
// METAL-NOT: visibility_0
