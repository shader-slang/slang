//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=EXTENDED): -experimental-feature -entry main -stage raygeneration -target metal -capability metal_raytracing_extended_limits

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.value = input.triangle.barycentricCoord.x + input.distance;
        input.payload.value += input.worldSpaceOrigin.x + input.worldSpaceDirection.y;
        if (!input.triangle.frontFacing || input.hitKind == HIT_KIND_TRIANGLE_BACK_FACE)
            input.ignoreHit();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data>
// METAL: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data)]]
// METAL-SAME: float distance_{{[0-9]+}} {{\[\[}}distance]]
// METAL-SAME: float2 barycentricCoord_{{[0-9]+}} {{\[\[}}barycentric_coord]]
// METAL-SAME: bool frontFacing_{{[0-9]+}} {{\[\[}}front_facing]]
// METAL-SAME: float3 worldSpaceOrigin_{{[0-9]+}} {{\[\[}}world_space_origin]]
// METAL-SAME: float3 worldSpaceDirection_{{[0-9]+}} {{\[\[}}world_space_direction]]
// METAL: barycentricCoord_{{[0-9]+}}.x
// METAL: frontFacing_

// EXTENDED: metal::raytracing::acceleration_structure<metal::raytracing::instancing> scene_{{[0-9]+}}
// EXTENDED: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data, metal::raytracing::extended_limits>
// EXTENDED: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data, metal::raytracing::extended_limits>
// EXTENDED: metal::raytracing::intersection_result<metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data, metal::raytracing::extended_limits>
// EXTENDED: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::triangle_data, metal::raytracing::world_space_data, metal::raytracing::extended_limits)]]
