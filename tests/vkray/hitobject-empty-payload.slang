// HitObject::TraceRay and TraceMotionRay reach the same payload ABI through intrinsic helpers.
// Their empty payload must remain a real RayPayloadKHR variable even with optimization disabled.
// Exercise both the NV and EXT data-based intrinsic helpers.

//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderNV -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderNV -O3 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O3 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderMotionNV -DUSE_MOTION=1 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderMotionEXT -DUSE_MOTION=1 -validate-ir

struct Empty
{
}

struct Payload
{
    Empty inner;
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> result;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;

    Payload payload;
#if USE_MOTION
    HitObject hit = HitObject::TraceMotionRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, 0.5, payload);
#else
    HitObject hit = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
#endif
    result[0] = hit.IsHit() ? 1 : 0;
}

// The logical payload disappears. The actual instruction still needs a global interface object
// with the same one-uint physical representation used by the other ray-tracing backends.
// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[carrier:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer RayPayloadKHR %[[carrier]]
// SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] RayPayloadKHR
// SPIRV: OpHitObjectTraceRay{{(Motion)?(NV|EXT)}} {{.*}} %[[payload]]
