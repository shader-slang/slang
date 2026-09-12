//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint mode;
    uint visitCount;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = HitRecord;
}

struct TestAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.visitCount++;
        if (input.payload.mode == 1)
            input.ignoreHit();
        if (input.payload.mode == 2)
            input.acceptHitAndEndSearch();
        input.payload.visitCount += 4;
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
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc;
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.tMin = 0.001;
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 1000.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 1;
    desc.missIndex = 0;

    TestPayload payload = { output[0], 0 };
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    output[1] = payload.visitCount;
}

// METAL: program_resources_1->intersectionFunctions_0
// METAL: _slang_intersector.intersect(
// METAL: *(&rayData_{{[0-9]+}}));
// METAL: struct StructuralRayTracingFilterResult
// METAL: bool accept_0 {{\[\[}}accept_intersection]];
// METAL: bool continueSearch_0 {{\[\[}}continue_search]];
// METAL: using namespace metal::raytracing;
// METAL: {{\[\[}}intersection(triangle, metal::raytracing::instancing)]]
// METAL-SAME: StructuralRayTracingFilterResult_0 __slang_structural_rt_{{[0-9a-f]+}}(
// METAL-SAME: ray_data Schema_rayData_0& rayData_{{[0-9]+}} {{\[\[}}payload]])
// METAL: (&(&rayDataStorage_0)->payload_{{[0-9]+}})->visitCount_0 =
// METAL: StructuralRayTracingFilterResult_0 {{.*}} = { false, true };
// METAL: StructuralRayTracingFilterResult_0 {{.*}} = { true, false };
// METAL: (&(&rayDataStorage_0)->payload_{{[0-9]+}})->visitCount_0 =
// METAL: rayData_{{[0-9]+}} = rayDataStorage_0;
