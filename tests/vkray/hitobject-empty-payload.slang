// HitObject::TraceRay reaches the same payload ABI through intrinsic helper functions. Its empty
// payload must remain a real RayPayloadKHR variable even when ordinary optimization is disabled.
// Exercise both the NV and EXT data-based intrinsic helpers.

//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderNV -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderNV -O3 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O3 -validate-ir

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
    HitObject hit = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
    result[0] = hit.IsHit() ? 1 : 0;
}

// SPIRV: OpMemberName %[[carrier:[A-Za-z0-9_]+]] 0 "_slang_dummy"
// SPIRV: %[[int:[A-Za-z0-9_]+]] = OpTypeInt 32 1
// SPIRV: %[[carrier]] = OpTypeStruct %[[int]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer RayPayloadKHR %[[carrier]]
// SPIRV: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] RayPayloadKHR
// SPIRV: OpHitObjectTraceRay{{NV|EXT}} {{.*}} %[[payload]]
