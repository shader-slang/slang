//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

rt::AccelerationStructure scene;
RWStructuredBuffer<uint> output;

struct Payload
{
    uint value;
}

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.TMin = 0.0;
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMax = 1000.0;

    Payload payload = { 0 };
    TraceRay(scene, 0, 0xff, 0, 1, 0, ray, payload);
    output[0] = payload.value;
}

// HLSL: RaytracingAccelerationStructure scene
// HLSL: TraceRay(scene
// SPIRV: OpTypeAccelerationStructureKHR
// SPIRV: OpTraceRayKHR
