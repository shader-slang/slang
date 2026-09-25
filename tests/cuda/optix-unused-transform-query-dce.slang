// Regression test for https://github.com/shader-slang/slang/issues/13262
//
// An unused ray-tracing transform query must be removed by Slang itself, so a dead
// `WorldToObject3x4()` compiles like no query at all. On CUDA the downstream compiler
// cannot drop it: the OptiX transform-list walk behind the query uses `asm volatile` loads.
// The SPIR-V and HLSL checks confirm the call is gone from Slang's own output, rather
// than relying on spirv-opt or DXC to remove it later.

//TEST:SIMPLE(filecheck=CUDA_DEAD): -target cuda -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=CUDA_LIVE): -target cuda -entry closestHitLive -stage closesthit
//TEST:SIMPLE(filecheck=SPIRV_DEAD): -target spirv-asm -O0 -entry closestHitDead -stage closesthit
//TEST:SIMPLE(filecheck=SPIRV_LIVE): -target spirv-asm -O0 -entry closestHitLive -stage closesthit
//TEST:SIMPLE(filecheck=HLSL_DEAD): -target hlsl -entry closestHitDead -stage closesthit

struct Payload
{
    float value;
};

// CUDA_DEAD: __closesthit__closestHitDead
// CUDA_DEAD-NOT: {{ObjectToWorld|WorldToObject}}
// CUDA_DEAD: optixSetPayload_0

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

// CUDA_LIVE-DAG: slangOptixGetObjectToWorldTransformMatrix()
// CUDA_LIVE-DAG: slangOptixGetWorldToObjectTransformMatrix()
// CUDA_LIVE-DAG: slangOptixGetObjectToWorldTransformMatrix4x3()
// CUDA_LIVE-DAG: slangOptixGetWorldToObjectTransformMatrix4x3()

// SPIRV_LIVE-DAG: BuiltIn ObjectToWorldKHR
// SPIRV_LIVE-DAG: BuiltIn WorldToObjectKHR
[shader("closesthit")]
void closestHitLive(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.value = attributes.barycentrics.x + ObjectToWorld3x4()[0][0] + WorldToObject3x4()[0][1]
                    + ObjectToWorld4x3()[0][2] + WorldToObject4x3()[1][0] + ObjectToWorld()[1][1]
                    + WorldToObject()[1][2];
}
