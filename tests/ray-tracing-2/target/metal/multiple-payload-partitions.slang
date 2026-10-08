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
        input.payload.visibility = input.attributes.frontFacing ? 1.0 : 0.0;
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

// Payload carriers remain typed, but both calls share one schema-wide set of tables. The union
// of stage requirements gives the shared IFT triangle_data and the shared closest-hit signature
// a distance argument, even though only the shadow shaders need those inputs.
// METAL-DAG: struct Schema_payload0_rayData_0
// METAL-DAG: struct Schema_payload1_rayData_0
// METAL-DAG: struct Schema_resources{{.*}}
// METAL-DAG: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data> intersectionFunctions_0;
// METAL-DAG: visible_function_table<void(void thread*)> missFunctions_0;
// METAL-DAG: visible_function_table<void(void thread*, float)> closestHitFunctions_0;
// METAL-DAG: visible_function_table<void(void thread*)> callableFunctions_0;
// METAL-DAG: uint device* records_0;

// The records buffer has one physical hit/miss stride across all payload views.
// METAL: thread Schema_payload1_rayData_0
// METAL: _slang_miss_record = {{.*}} * int(48)
// METAL: _slang_hit_record = {{.*}} * int(48)
// METAL: thread Schema_payload0_rayData_0
// METAL: _slang_miss_record = {{.*}} * int(48)
// METAL: _slang_hit_record = {{.*}} * int(48)

// The single triangle dispatcher selects a typed carrier by the schema-wide functionIndex,
// copies it from ray_data into thread storage, then writes changes back to that same carrier.
// METAL-LABEL: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::triangle_data)]]
// METAL-SAME: ray_data StructuralRayDataHeader_0&
// METAL: case int(1):
// METAL: Schema_payload1_rayData_0 ray_data*
// METAL: thread Schema_payload1_rayData_0
// METAL: radiance_0 =
// METAL: case int(0):
// METAL: Schema_payload0_rayData_0 ray_data*
// METAL: thread Schema_payload0_rayData_0
// METAL: visibility_0 =
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing, metal::raytracing::triangle_data)]]
// METAL-NOT: intersection(triangle,
