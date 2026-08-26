//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint recordValue;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint recordValue;
}

struct RuntimeRecord
{
    uint value;
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
    typealias Record = RuntimeRecord;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Record = RuntimeRecord;
}

struct ClosestHit0 : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 10;
        input.payload.recordValue = input.record.value;
    }
}

struct ClosestHit1 : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 11;
        input.payload.recordValue = input.record.value;
    }
}

struct Miss0 : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 20;
        input.payload.recordValue = input.record.value;
    }
}

struct Miss1 : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 21;
        input.payload.recordValue = input.record.value;
    }
}

struct HitGroup0 : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = ClosestHit0;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct HitGroup1 : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = HitContext;
    typealias ClosestHit = ClosestHit1;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissGroup0 : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = Miss0;
}

struct MissGroup1 : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<1>;
    typealias Context = MissContext;
    typealias Miss = Miss1;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, HitGroup0, HitGroup1>;
    typealias MissGroups = rt::MissGroupList<RuntimeTraceContext, MissGroup0, MissGroup1>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    uint logicalSlot = rayIndex & 1;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex < 2 ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = logicalSlot;
    desc.sbtStride = 0;
    desc.missIndex = logicalSlot;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].recordValue = payload.recordValue;
    results[rayIndex].dispatchWidth = DispatchRaysDimensions().x;
}

// HLSL-DAG: [shader("closesthit")]void ClosestHit0(
// HLSL-DAG: [shader("closesthit")]void ClosestHit1(
// HLSL-DAG: [shader("miss")]void Miss0(
// HLSL-DAG: [shader("miss")]void Miss1(
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "ClosestHit0"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "ClosestHit1"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "Miss0"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "Miss1"
// METAL-DAG: metal::visible_function_table<void(ProgramLayout_rayData_0 thread*)> missFunctions_0;
// METAL-DAG: metal::visible_function_table<void(ProgramLayout_rayData_0 thread*)> closestHitFunctions_0;
// METAL-DAG: {{\[\[visible\]\]}} void ClosestHit0_0
// METAL-DAG: {{\[\[visible\]\]}} void ClosestHit1_0
// METAL-DAG: {{\[\[visible\]\]}} void Miss0_0
// METAL-DAG: {{\[\[visible\]\]}} void Miss1_0
