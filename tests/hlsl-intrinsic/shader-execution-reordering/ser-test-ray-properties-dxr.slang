// ser-test-ray-properties-dxr.slang
// Test DXR 1.3-only Ray Property Methods

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

RWStructuredBuffer<float> outputBuffer;
RaytracingAccelerationStructure accelStruct;

struct MyPayload { float3 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload;

    HitObject hitObj = HitObject::TraceRay(
        accelStruct, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    float result = 0.0f;

    // DXR 1.3-only methods:

    // GetRayFlags - returns ray flags
    uint rayFlags = hitObj.GetRayFlags();
    result += float(rayFlags);

    // GetRayTMin - returns TMin parameter
    float tmin = hitObj.GetRayTMin();
    result += tmin;

    // GetRayTCurrent - returns parametric hit distance
    float tcurrent = hitObj.GetRayTCurrent();
    result += tcurrent;

    // GetWorldRayOrigin - returns world-space ray origin
    float3 worldOrigin = hitObj.GetWorldRayOrigin();
    result += worldOrigin.x;

    // GetWorldRayDirection - returns world-space ray direction
    float3 worldDir = hitObj.GetWorldRayDirection();
    result += worldDir.y;

    outputBuffer[idx] = result;
}
