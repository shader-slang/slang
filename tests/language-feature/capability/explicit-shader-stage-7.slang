//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry main -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_WARN): -target spirv -entry main -profile sm_5_0 -DWARN

// The explicit ser_motion_raygen stage supplies the motion-blur and SER capabilities, so
// TraceMotionRay does not need a capability-upgrade warning. Its ray-tracing requirement
// also contributes SPIR-V 1.4, even though the requested profile is the HLSL sm_5_0 profile.
//CHECK_PASS-NOT: warning[E41012]
//CHECK_PASS-NOT: warning[E50011]
//CHECK_PASS: Version: 1.4
//CHECK_PASS-NOT: warning[E41012]
//CHECK_PASS-NOT: warning[E50011]
//CHECK_PASS: OpEntryPoint RayGenerationKHR %main "main"
//CHECK_PASS-NOT: warning[E41012]
//CHECK_PASS-NOT: warning[E50011]

// The ordinary raygen stage still needs the capability-upgrade warning for motion blur.
// Both variants honor the canonical SPIR-V version requirement, so neither emits the
// obsolete warning about a version below 1.3.
//CHECK_WARN-NOT: warning[E50011]
//CHECK_WARN: warning[E41012]
//CHECK_WARN-NOT: warning[E50011]
//CHECK_WARN: Version: 1.4
//CHECK_WARN-NOT: warning[E50011]
//CHECK_WARN: OpEntryPoint RayGenerationKHR %main "main"
//CHECK_WARN-NOT: warning[E50011]

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
