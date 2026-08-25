//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -target hlsl -profile sm_6_9 -capability ser_dxr -entry main -stage raygeneration

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

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups<::TraceContext>;
    typealias MissGroups = rt::NoMissGroups<::TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<::TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    Payload payload = {};
    HitObject hit = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xff,
        0,
        0,
        0,
        desc.ray,
        payload);

    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = hit.IsHit() ? payload.value : 0;
}

// CHECK: dx::HitObject::TraceRay
// CHECK: TraceRay(
// CHECK: .IsHit
