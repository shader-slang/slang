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
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = 2;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct FrameParameters
{
    rt::AccelerationStructure scene;
    rt::TraceProgramDescriptor<Schema> program;
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
    rt::RayTracer<Schema> tracer;
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
