// Regression test for https://github.com/shader-slang/slang/issues/13262
//
// Slang removes an unused ray-tracing transform query itself, so a dead `WorldToObject3x4()`
// compiles the same as no query. NVRTC cannot remove it after emission, because the OptiX
// transform-list walk behind the query loads through `asm volatile`. The SPIR-V and HLSL checks
// show the call is already gone from Slang's output rather than left for spirv-opt or DXC, and
// the PTX check shows no transform-list walk reaches the compiled code.

//TEST:SIMPLE(filecheck=CUDA_DEAD): -target cuda -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=CUDA_LIVE): -target cuda -entry closestHitLive -stage closesthit
//TEST:SIMPLE(filecheck=PTX_DEAD): -target ptx -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=PTX_LIVE): -target ptx -entry closestHitLive -stage closesthit
//TEST:SIMPLE(filecheck=SPIRV_DEAD): -target spirv-asm -O0 -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=SPIRV_LIVE): -target spirv-asm -O0 -entry closestHitLive -stage closesthit
//TEST:SIMPLE(filecheck=HLSL_DEAD): -target hlsl -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=HLSL_LIVE): -target hlsl -entry closestHitLive -stage closesthit

struct Payload
{
    float value;
};

// CUDA_DEAD: __closesthit__closestHitDead
// CUDA_DEAD-NOT: {{ObjectToWorld|WorldToObject}}
// CUDA_DEAD: optixSetPayload_0

// PTX_DEAD: .version
// PTX_DEAD-NOT: _optix_get_transform_list

// SPIRV_DEAD: OpEntryPoint
// SPIRV_DEAD-NOT: {{ObjectToWorldKHR|WorldToObjectKHR}}

// HLSL_DEAD: closestHitDead
// HLSL_DEAD-NOT: {{ObjectToWorld|WorldToObject}}
// HLSL_DEAD: barycentrics
[shader("closesthit")]
void closestHitDead(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    float3x4 objectToWorld3x4 = ObjectToWorld3x4();
    float3x4 worldToObject3x4 = WorldToObject3x4();
    float4x3 objectToWorld4x3 = ObjectToWorld4x3();
    float4x3 worldToObject4x3 = WorldToObject4x3();
    float3x4 objectToWorld = ObjectToWorld();
    float3x4 worldToObject = WorldToObject();
    payload.value = attributes.barycentrics.x;
}

// CUDA_LIVE: __closesthit__closestHitLive
// CUDA_LIVE-DAG: slangOptixGetObjectToWorldTransformMatrix()
// CUDA_LIVE-DAG: slangOptixGetWorldToObjectTransformMatrix()
// CUDA_LIVE-DAG: slangOptixGetObjectToWorldTransformMatrix4x3()
// CUDA_LIVE-DAG: slangOptixGetWorldToObjectTransformMatrix4x3()
// CUDA_LIVE-DAG: {{ObjectToWorld_[0-9]+}}()
// CUDA_LIVE-DAG: {{WorldToObject_[0-9]+}}()

// PTX_LIVE: _optix_get_transform_list

// SPIRV_LIVE-DAG: BuiltIn ObjectToWorldKHR
// SPIRV_LIVE-DAG: BuiltIn WorldToObjectKHR

// HLSL_LIVE: closestHitLive
// HLSL_LIVE-DAG: ObjectToWorld3x4()
// HLSL_LIVE-DAG: WorldToObject3x4()
// HLSL_LIVE-DAG: ObjectToWorld4x3()
// HLSL_LIVE-DAG: WorldToObject4x3()
// HLSL_LIVE-DAG: {{ObjectToWorld_[0-9]+}}()
// HLSL_LIVE-DAG: {{WorldToObject_[0-9]+}}()
[shader("closesthit")]
void closestHitLive(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = attributes.barycentrics.x + ObjectToWorld3x4()[0][0] + WorldToObject3x4()[0][1]
                    + ObjectToWorld4x3()[0][2] + WorldToObject4x3()[1][0] + ObjectToWorld()[1][1]
                    + WorldToObject()[1][2];
}
