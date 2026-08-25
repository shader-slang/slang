//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPIRV_CLOSEST): -experimental-feature -entry RuntimeClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// HLSL: TraceRay(
// HLSL: [shader("raygeneration")]void main()
// HLSL: TraceRay(
// HLSL: [shader("miss")]void RuntimeMiss(
// HLSL: [shader("closesthit")]void RuntimeClosestHit(
// SPIRV-DAG: OpEntryPoint RayGenerationKHR {{.*}} "main"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// SPIRV-COUNT-2: OpTraceRayKHR
// SPIRV_CLOSEST: OpEntryPoint ClosestHitKHR {{.*}} "main"
// SPIRV_CLOSEST: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// METAL-COUNT-2: .intersect(
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeClosestHit
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeMiss

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint depth;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint depth;
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
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

void traceSecondary(inout RuntimePayload payload);

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        if (input.payload.depth == 0)
        {
            input.payload.depth = 1;
            traceSecondary(input.payload);
        }
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
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
        input.payload.stage = 20 + input.payload.depth;
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

rt::RayTraversalDesc makeTraceDesc(float3 origin)
{
    rt::RayTraversalDesc desc = {};
    desc.ray.origin = origin;
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 0;
    desc.missIndex = 0;
    return desc;
}

void traceSecondary(inout RuntimePayload payload)
{
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(makeTraceDesc(float3(2.0, 2.0, 0.0)), scene, program, payload);
}

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    float3 origin = rayIndex == 0 ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    tracer.trace(makeTraceDesc(origin), scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].depth = payload.depth;
    results[rayIndex].dispatchWidth = DispatchRaysDimensions().x;
}
