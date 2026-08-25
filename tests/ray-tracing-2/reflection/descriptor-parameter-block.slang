//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:REFLECTION(filecheck=REFLECT): -experimental-feature -entry main -stage raygeneration -target metal -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
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
        input.payload.value = 2;
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

struct FrameParameters
{
    rt::AccelerationStructure scene;
    rt::TraceProgramDescriptor<ProgramLayout> program;
    RWStructuredBuffer<uint> output;
}

ParameterBlock<FrameParameters> frame;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    Payload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, frame.scene, frame.program, payload);
    frame.output[0] = payload.value;
}

// HLSL-DAG: RaytracingAccelerationStructure frame_scene
// HLSL-DAG: RWStructuredBuffer<uint > frame_output
// HLSL-DAG: [shader("closesthit")]void ClosestHit(
// HLSL-DAG: [shader("miss")]void Miss(
// SPIRV-DAG: OpEntryPoint ClosestHitKHR {{.*}} "ClosestHit"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "Miss"
// METAL-DAG: struct FrameParameters_default_0
// METAL-DAG: metal::raytracing::acceleration_structure<metal::raytracing::instancing> scene_0;
// METAL-DAG: rt_TraceProgramDescriptor_default_0 program_0;
// METAL-DAG: rt_TraceProgramDescriptorResources_default_0 constant* resources_0;
// METAL-DAG: uint device* output_0;
// REFLECT: "name": "frame"
// REFLECT: "kind": "parameterBlock"
// REFLECT: "name": "FrameParameters"
// REFLECT-DAG: "name": "scene"
// REFLECT-DAG: "baseShape": "accelerationStructure"
// REFLECT-DAG: "name": "program"
// REFLECT-DAG: "name": "TraceProgramDescriptor"
// REFLECT-DAG: "name": "resources"
// REFLECT-DAG: "kind": "parameterBlock"
// REFLECT-DAG: "name": "output"
