//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=METAL_CLOSEST): -experimental-feature -entry RuntimeClosestHit -stage closesthit -target metal
//TEST:SIMPLE(filecheck=METAL_MISS): -experimental-feature -entry RuntimeMiss -stage miss -target metal

//HLSL: [shader("raygeneration")]void main()
//HLSL: [shader("miss")]void RuntimeMiss(
//HLSL: [shader("closesthit")]void RuntimeClosestHit(
//SPIRV: OpEntryPoint RayGenerationKHR {{.*}} "main"
//SPIRV: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
//SPIRV: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
//METAL-DAG: thread_position_in_grid
//METAL-DAG: threads_per_grid
//METAL-NOT: DispatchRaysIndex
//METAL-NOT: DispatchRaysDimensions
//METAL_CLOSEST: void RuntimeClosestHit_invoke_0(RuntimePayload_0 thread* payload_0)
//METAL_CLOSEST: payload_0->stage_0 = 1U;
//METAL_MISS: void RuntimeMiss_invoke_0(RuntimePayload_0 thread* payload_0)
//METAL_MISS: payload_0->stage_0 = 2U;

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint primitiveIndex;
    uint dispatchWidth;
}

struct RuntimePayload
{
    uint stage;
    uint primitiveIndex;
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

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 1;
        input.payload.primitiveIndex = 0;
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
        input.payload.stage = 2;
        input.payload.primitiveIndex = 0xffffffff;
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

struct FrameParameters
{
    rt::AccelerationStructure scene;
    rt::TraceProgramDescriptor<ProgramLayout> program;
    RWStructuredBuffer<RuntimeResult> results;
}

ParameterBlock<FrameParameters> frame;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;
    uint dispatchWidth = DispatchRaysDimensions().x;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.25, 0.25, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.time = 0.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 0;
    desc.missIndex = 0;

    RuntimePayload payload = { 0, 0 };
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, frame.scene, frame.program, payload);

    frame.results[rayIndex].stage = payload.stage;
    frame.results[rayIndex].primitiveIndex = payload.primitiveIndex;
    frame.results[rayIndex].dispatchWidth = dispatchWidth;
}
