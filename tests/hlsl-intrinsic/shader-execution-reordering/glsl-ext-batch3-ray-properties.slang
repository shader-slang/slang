//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage raygeneration -entry main -capability GL_EXT_shader_invocation_reorder

// BATCH 3: Ray Property Getters - EXT Version
// Tests 5 ray property getter functions

//CHECK: OpCapability ShaderInvocationReorderEXT
//CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
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

    // Create a hit object with known ray parameters
    HitObject hitObj = HitObject::TraceRay(
        scene,
        RAY_FLAG_NONE,
        0xFF,
        0, 1, 0,
        ray,
        payload);

    float result = 0.0;

    // =================================================================
    // Function 11: hitObjectGetRayTMinEXT → OpHitObjectGetRayTMinEXT
    // =================================================================
    // CHECK: OpHitObjectGetRayTMinEXT
    if (!hitObj.IsNop())
    {
        float tmin = hitObj.GetRayTMin();
        result += tmin; // Should be 0.5
    }

    // =================================================================
    // Function 12: hitObjectGetRayTMaxEXT → OpHitObjectGetRayTMaxEXT
    // =================================================================
    // CHECK: OpHitObjectGetRayTMaxEXT
    if (!hitObj.IsNop())
    {
        RayDesc rayDesc = hitObj.GetRayDesc();
        float tmax = rayDesc.TMax;
        result += tmax * 0.01; // Should be 100.0 * 0.01 = 1.0
    }

    // =================================================================
    // Function 13: hitObjectGetRayFlagsEXT → OpHitObjectGetRayFlagsEXT
    // =================================================================
    // CHECK: OpHitObjectGetRayFlagsEXT
    if (!hitObj.IsNop())
    {
        uint rayFlags = hitObj.GetRayFlags();
        result += float(rayFlags); // Should be 0
    }

    // =================================================================
    // Function 14: hitObjectGetWorldRayOriginEXT → OpHitObjectGetWorldRayOriginEXT
    // =================================================================
    // CHECK: OpHitObjectGetWorldRayOriginEXT
    if (!hitObj.IsNop())
    {
        float3 worldOrigin = hitObj.GetWorldRayOrigin();
        result += worldOrigin.x; // Should be 1.0
        result += worldOrigin.y; // Should be 2.0
        result += worldOrigin.z; // Should be 3.0
    }

    // =================================================================
    // Function 15: hitObjectGetWorldRayDirectionEXT → OpHitObjectGetWorldRayDirectionEXT
    // =================================================================
    // CHECK: OpHitObjectGetWorldRayDirectionEXT
    if (!hitObj.IsNop())
    {
        float3 worldDir = hitObj.GetWorldRayDirection();
        result += worldDir.z; // Should be 1.0
    }

    // Total expected: 0.5 (tmin) + 1.0 (tmax*0.01) + 0 (flags) + 1.0+2.0+3.0 (origin) + 1.0 (dir.z) = 8.5
    outputBuffer[idx] = result;
}
