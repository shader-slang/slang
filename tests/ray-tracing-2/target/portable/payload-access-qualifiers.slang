//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry main -stage raygeneration -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

[raypayload]
struct TestPayload
{
    float callerToHit : read(anyhit, closesthit, miss) : write(caller);
    float hitToCaller : read(caller) : write(closesthit, miss);
    float anyHitState : read(caller, anyhit) : write(caller, anyhit);
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        if (input.payload.callerToHit < 0.0)
            input.ignoreHit();
        input.payload.anyHitState++;
    }
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.hitToCaller = input.payload.callerToHit;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = ::TraceContext;
    typealias Record = void;
}

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.hitToCaller = 0.0;
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = ::Miss;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<::TraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<::TraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<::TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<float> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    TestPayload payload = { 1.0, 0.0, 0.0 };
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = payload.hitToCaller + payload.anyHitState;
}

// HLSL-DAG: struct [raypayload] TestPayload
// HLSL-DAG: float callerToHit_0 : read(anyhit, closesthit, miss) : write(caller);
// HLSL-DAG: float hitToCaller_0 : read(caller) : write(closesthit, miss);
// HLSL-DAG: float anyHitState_0 : read(caller, anyhit) : write(caller, anyhit);
// HLSL-DAG: [shader("anyhit")]void AnyHit(
// HLSL-DAG: [shader("closesthit")]void ClosestHit(
// HLSL-DAG: [shader("miss")]void Miss(
// DXIL: !dx.dxrPayloadAnnotations
// SPIRV-DAG: OpEntryPoint AnyHitKHR {{.*}} "AnyHit"
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "ClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "Miss"
// SPIRV-DAG: IncomingRayPayloadKHR
