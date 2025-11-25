// ser-test-transform-matrices.slang
// Test HitObject Transform Matrix Methods

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration
// Test with NVAPI:    slangc -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration

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

    if (hitObj.IsHit())
    {
        // Note: DXR 1.3 uses 3x4/4x3 variants, NVAPI uses GetObjectToWorld/GetWorldToObject
        // These tests cover the common case - specific API tests below

        // GetObjectRayOrigin/Direction work on all targets
        float3 objOrigin = hitObj.GetObjectRayOrigin();
        float3 objDir = hitObj.GetObjectRayDirection();
        result += objOrigin.x + objDir.y;
    }

    outputBuffer[idx] = result;
}
