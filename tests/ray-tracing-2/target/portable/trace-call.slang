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
    typealias Payload = MyPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::NoHitGroups<MyTraceContext>;
    typealias MissGroups = rt::NoMissGroups<MyTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<MyTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<float> output;

void traceHelper(rt::RayTraversalDesc desc, inout MyPayload payload)
{
    rt::RayTracer<ProgramLayout> tracer;
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

// IR: struct [[PROGRAM_LAYOUT:%ProgramLayout]] : Type
// IR: structuralRayTracingTrace({{.*}}, [[PROGRAM_LAYOUT]],
// HLSL: TraceRay(
// HLSL-SAME: .rayFlags
// SPIRV: OpTraceRayKHR
// METAL: {{\[\[kernel\]\]}} void main_0(
// METAL-NOT: acceleration_structure
// METAL-NOT: TraceProgramDescriptor
