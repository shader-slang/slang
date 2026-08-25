//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint anyHitCount;
    uint barycentricX;
    uint barycentricY;
    uint frontFacing;
    uint flagsMatch;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint mode;
    uint anyHitCount;
    uint barycentricX;
    uint barycentricY;
    uint frontFacing;
    uint expectedFlags;
    uint flagsMatch;
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

struct RuntimeAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        let triangle = input.triangle;
        input.payload.stage = 1;
        input.payload.anyHitCount++;
        input.payload.barycentricX = uint(triangle.barycentricCoord.x * 100.0 + 0.5);
        input.payload.barycentricY = uint(triangle.barycentricCoord.y * 100.0 + 0.5);
        input.payload.frontFacing = triangle.frontFacing ? 1 : 0;
        input.payload.flagsMatch = input.rayFlags == input.payload.expectedFlags ? 1 : 0;
        if (input.payload.mode == 1)
        {
            input.ignoreHit();
            return;
        }
        if (input.payload.mode == 2)
        {
            input.acceptHitAndEndSearch();
            return;
        }
    }
}

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        let triangle = input.triangle;
        input.payload.stage = 3;
        input.payload.barycentricX = uint(triangle.barycentricCoord.x * 100.0 + 0.5);
        input.payload.barycentricY = uint(triangle.barycentricCoord.y * 100.0 + 0.5);
        input.payload.frontFacing = triangle.frontFacing ? 1 : 0;
        input.payload.flagsMatch = input.rayFlags == input.payload.expectedFlags ? 1 : 0;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = RuntimeAnyHit;
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
        input.payload.stage = 2;
        input.payload.flagsMatch = input.rayFlags == input.payload.expectedFlags ? 1 : 0;
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
    uint flags = RAY_FLAG_NONE;
    uint mode = 0;
    if (rayIndex == 1)
        flags = RAY_FLAG_SKIP_TRIANGLES;
    else if (rayIndex == 2)
    {
        flags = RAY_FLAG_FORCE_NON_OPAQUE;
        mode = 1;
    }
    else if (rayIndex == 3)
    {
        flags = RAY_FLAG_FORCE_NON_OPAQUE;
        mode = 2;
    }
    else if (rayIndex == 4)
        flags = RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    else if (rayIndex == 5)
        flags = RAY_FLAG_CULL_OPAQUE;
    else if (rayIndex == 6)
        flags = RAY_FLAG_CULL_FRONT_FACING_TRIANGLES;
    else if (rayIndex == 7)
        flags = RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    else if (rayIndex == 8)
        flags = RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    else if (rayIndex == 9)
    {
        flags = RAY_FLAG_FORCE_OPAQUE;
        mode = 1;
    }

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = float3(0.25, 0.25, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = flags;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 0;
    desc.missIndex = 0;

    RuntimePayload payload = {};
    payload.stage = 40;
    payload.mode = mode;
    payload.expectedFlags = flags;
    payload.flagsMatch = 1;
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].anyHitCount = payload.anyHitCount;
    results[rayIndex].barycentricX = payload.barycentricX;
    results[rayIndex].barycentricY = payload.barycentricY;
    results[rayIndex].frontFacing = payload.frontFacing;
    results[rayIndex].flagsMatch = payload.flagsMatch;
    results[rayIndex].dispatchWidth = DispatchRaysDimensions().x;
}

// HLSL-DAG: [shader("anyhit")]void RuntimeAnyHit(
// HLSL-DAG: [shader("closesthit")]void RuntimeClosestHit(
// HLSL-DAG: [shader("miss")]void RuntimeMiss(
// HLSL-DAG: IgnoreHit()
// HLSL-DAG: AcceptHitAndEndSearch()
// SPIRV-DAG: OpEntryPoint AnyHitKHR {{.*}} "RuntimeAnyHit"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
// SPIRV-DAG: OpIgnoreIntersectionKHR
// SPIRV-DAG: OpTerminateRayKHR
// METAL-DAG: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::triangle_data){{\]\]}}
// METAL-DAG: force_opacity
// METAL-DAG: set_triangle_cull_mode
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeClosestHit
// METAL-DAG: {{\[\[visible\]\]}} void RuntimeMiss
