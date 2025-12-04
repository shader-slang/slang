//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 4: Object Space Ray Getters - EXT Version
// Tests 5 object space transformation and ray property functions

//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct [raypayload] RayPayload
{
    float3 color : read(caller, closesthit, miss) : write(caller, closesthit, miss);
};

[shader("raygeneration")]
void main()
{
    uint2 launchID = DispatchRaysIndex().xy;
    uint idx = launchID.x;

    RayDesc ray;
    ray.Origin = float3(1.0, 2.0, 3.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.5;
    ray.TMax = 100.0;

    RayPayload payload;
    payload.color = float3(0, 0, 0);

    HitObject hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        ray,
        payload);

    float result = 0.0;

    // =================================================================
    // Function 16: hitObjectGetObjectRayOriginEXT → OpHitObjectGetObjectRayOriginEXT
    // =================================================================
    // CHECK: OpHitObjectGetObjectRayOriginEXT
    if (hitObj.IsHit())
    {
        float3 objOrigin = hitObj.GetObjectRayOrigin();
        result += objOrigin.x;
    }

    // =================================================================
    // Function 17: hitObjectGetObjectRayDirectionEXT → OpHitObjectGetObjectRayDirectionEXT
    // =================================================================
    // CHECK: OpHitObjectGetObjectRayDirectionEXT
    if (hitObj.IsHit())
    {
        float3 objDir = hitObj.GetObjectRayDirection();
        result += objDir.z;
    }

    // =================================================================
    // Function 18: hitObjectGetObjectToWorldEXT → OpHitObjectGetObjectToWorldEXT
    // =================================================================
    // CHECK: OpHitObjectGetObjectToWorldEXT
    if (hitObj.IsHit())
    {
        float4x3 o2w = hitObj.GetObjectToWorld();
        result += o2w[0][0];
    }

    // =================================================================
    // Function 19: hitObjectGetWorldToObjectEXT → OpHitObjectGetWorldToObjectEXT
    // =================================================================
    // CHECK: OpHitObjectGetWorldToObjectEXT
    if (hitObj.IsHit())
    {
        float4x3 w2o = hitObj.GetWorldToObject();
        result += w2o[0][0];
    }

    // =================================================================
    // Function 20: GetIntersectionTriangleVertexPositionsEXT (skipped - needs position fetch capability)
    // =================================================================
    // Note: Requires SPV_KHR_ray_tracing_position_fetch extension
    // Will test separately if needed

    outputBuffer[idx] = result;
}
