//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestAnyHit : rt::IAnyHitShader<HitContext>
{
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
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
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
