// ser-test-ray-properties-nvapi.slang
// Test NVAPI-only Ray Property Methods

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

    // NVAPI-only method:

    // GetRayDesc - returns a RayDesc struct (NVAPI only, not in DXR 1.3)
    RayDesc rayDesc = hitObj.GetRayDesc();
    result += rayDesc.Origin.x;
    result += rayDesc.TMin;
    result += rayDesc.Direction.y;
    result += rayDesc.TMax;

    outputBuffer[idx] = result;
}
