//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry main -allow-glsl -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_WARN): -target spirv -entry main -allow-glsl -profile sm_5_0 -DWARN

// A 'ser_motion_raygen' shader implicitly requires "raygen + raytracing_motion + ser", which pulls
// in SPIR-V 1.4. The via-GLSL legalization path records that as an internal `_spirv_1_4`
// IRRequireCapabilityAtomDecoration and the backend version scan honors it, so the emitted header is
// 1.4 — at or above the 1.3 minimum — and no E50011 "below spirv 1.3" warning is emitted here.
//CHECK_PASS: main

// Without 'ser_motion_raygen' the plain 'raygen' stage triggers the profile-upgrade warning.
//CHECK_WARN: warning[E41012]


struct RayPayload
{
    uint data;
};

uniform RaytracingAccelerationStructure sceneBVH;
RWStructuredBuffer<uint> outputBuffer;
#ifndef WARN
[shader("ser_motion_raygen")]
#else
[shader("raygen")]
#endif
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = {};
    TraceMotionRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, 0, payload);

    outputBuffer[0] = payload.data;
}
