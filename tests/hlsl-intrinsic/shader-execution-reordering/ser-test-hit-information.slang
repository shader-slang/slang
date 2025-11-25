// ser-test-hit-information.slang
// Test HitObject Hit Information Methods

// Test with DXR 1.3:  slangc -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration
// Test with NVAPI:    slangc -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration

RWStructuredBuffer<uint> outputBuffer;
RaytracingAccelerationStructure accelStruct;

struct MyPayload { float3 color; };
struct MyAttribs { float2 barycentrics; };

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

    uint result = 0;

    if (hitObj.IsHit())
    {
        // GetInstanceIndex - returns the instance index
        uint instanceIndex = hitObj.GetInstanceIndex();
        result += instanceIndex;

        // GetInstanceID - returns the instance ID (user-defined)
        uint instanceID = hitObj.GetInstanceID();
        result += instanceID;

        // GetGeometryIndex - returns geometry index within the instance
        uint geomIndex = hitObj.GetGeometryIndex();
        result += geomIndex;

        // GetPrimitiveIndex - returns primitive index within the geometry
        uint primIndex = hitObj.GetPrimitiveIndex();
        result += primIndex;

        // GetHitKind - returns the hit kind (front/back face, etc.)
        uint hitKind = hitObj.GetHitKind();
        result += hitKind;

        // GetAttributes - returns the hit attributes
        MyAttribs attribs = hitObj.GetAttributes<MyAttribs>();
        result += uint(attribs.barycentrics.x * 100);
    }

    outputBuffer[idx] = result;
}
