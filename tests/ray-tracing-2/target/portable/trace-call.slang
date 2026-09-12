//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -dump-ir -no-codegen

import slang.raytracing;

struct MyPayload
{
    float value;
}

struct MyTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MyMissContext : rt::IPayloadContext
{
    typealias TraceContext = MyTraceContext;
    typealias Payload = MyPayload;
    typealias Record = void;
}

struct MyMiss : rt::IMissShader
{
    typealias Context = MyMissContext;
    void invoke(rt::MissInput<MyMissContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<MyMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<float> output;

void traceHelper(rt::RayTraversalDesc desc, inout MyPayload payload)
{
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc;
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.tMin = 0.001;
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 1000.0;
    desc.rayFlags = RAY_FLAG_FORCE_OPAQUE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 1;
    desc.sbtStride = 2;
    desc.missIndex = 3;

    MyPayload payload = { 0.0 };
    traceHelper(desc, payload);
    output[0] = payload.value;
}

// IR: struct [[PROGRAM_SCHEMA:%Schema]] : Type
// IR: structuralRayTracingTrace({{.*}}, [[PROGRAM_SCHEMA]], "Schema",
// HLSL: TraceRay(
// HLSL-SAME: .rayFlags
// SPIRV: OpTraceRayKHR
// METAL: {{\[\[kernel\]\]}} void main_0(
// METAL-SAME: metal::raytracing::acceleration_structure
// METAL-SAME: rt_TraceProgramDescriptorResources
// METAL: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(
