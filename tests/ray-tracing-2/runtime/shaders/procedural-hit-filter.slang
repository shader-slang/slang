//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// HLSL-DAG: [shader("intersection")]void RuntimeIntersection()
// HLSL-DAG: [shader("anyhit")]void RuntimeAnyHit(
// HLSL-DAG: [shader("closesthit")]void RuntimeClosestHit(
// HLSL-DAG: [shader("miss")]void RuntimeMiss(
// HLSL-DAG: rt_IntersectionInput_reportHit_0(1.0f
// HLSL-DAG: rt_IntersectionInput_reportHit_0(1.5f
// HLSL-DAG: IgnoreHit()
// SPIRV-DAG: OpEntryPoint IntersectionKHR {{.*}} "RuntimeIntersection"
// SPIRV-DAG: OpEntryPoint AnyHitKHR {{.*}} "RuntimeAnyHit"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// SPIRV-DAG: OpIgnoreIntersectionKHR
// SPIRV-COUNT-2: OpReportIntersectionKHR
// METAL-DAG: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
// METAL-DAG: bool hasCandidate_0;
// METAL-DAG: candidateDistance_0 = 1.0f;
// METAL-DAG: candidateDistance_0 = 1.5f;
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeClosestHit
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeMiss

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint candidate;
    uint anyHitCount;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint candidate;
    uint anyHitCount;
}

struct RuntimeAttributes
{
    uint candidate;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias Payload = RuntimePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<RuntimeAttributes>;
    typealias Record = void;
}

struct RuntimeIntersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        RuntimeAttributes attributes = { 7 };
        input.reportHit(1.0, attributes);
        attributes.candidate = 9;
        input.reportHit(1.5, attributes);
    }
}

struct RuntimeAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.anyHitCount++;
        if (input.attributes.candidate == 7)
        {
            input.ignoreHit();
            return;
        }
    }
}

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 3;
        input.payload.candidate = input.attributes.candidate;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = RuntimeAnyHit;
    typealias Intersection = RuntimeIntersection;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Record = void;
}

struct RuntimeMiss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 2;
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = RuntimeMiss;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<RuntimeTraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    uint dispatchWidth = DispatchRaysDimensions().x;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.0, 0.0, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 0;
    desc.missIndex = 0;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].candidate = payload.candidate;
    results[rayIndex].anyHitCount = payload.anyHitCount;
    results[rayIndex].dispatchWidth = dispatchWidth;
}
