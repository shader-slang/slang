//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint mode;
    uint visitCount;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = HitRecord;
}

struct TestAnyHit : rt::IAnyHitShader<HitContext>
{
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
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
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
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    output[1] = payload.visitCount;
}

// METAL: _slang_intersector.intersect(
// METAL: program_resources_1->intersectionFunctions_0
// METAL: *(&payload_{{[0-9]+}}));
// METAL: struct StructuralRayTracingCandidateResult
// METAL: bool accept_0 {{\[\[}}accept_intersection]];
// METAL: bool continueSearch_0 {{\[\[}}continue_search]];
// METAL: using namespace metal::raytracing;
// METAL: {{\[\[}}intersection(triangle, metal::raytracing::instancing)]]
// METAL-SAME: StructuralRayTracingCandidateResult_0 HitGroup_candidate_0(
// METAL-SAME: ray_data TestPayload_0& payload_{{[0-9]+}} {{\[\[}}payload]])
// METAL: (&payloadStorage_0)->visitCount_0 =
// METAL: StructuralRayTracingCandidateResult_0 {{.*}} = { false, true };
// METAL: StructuralRayTracingCandidateResult_0 {{.*}} = { true, false };
// METAL: (&payloadStorage_0)->visitCount_0 =
// METAL: payload_{{[0-9]+}} = payloadStorage_0;
