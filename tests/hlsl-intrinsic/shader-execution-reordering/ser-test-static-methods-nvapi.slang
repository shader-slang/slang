// ser-test-static-methods-nvapi.slang
// Test HitObject Static Methods for NVAPI path

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
    payload.color = float3(0, 0, 0);

    MyAttribs attribs;
    attribs.barycentrics = float2(0.25f, 0.25f);

    // 1. TraceRay - creates HitObject by tracing a ray
    HitObject hitFromTrace = HitObject::TraceRay(
        accelStruct,
        RAY_FLAG_NONE,
        0xFF,
        0,  // RayContributionToHitGroupIndex
        1,  // MultiplierForGeometryContributionToHitGroupIndex
        0,  // MissShaderIndex
        ray,
        payload);

    // 2. MakeHit (computed index) - creates HitObject representing a hit
    HitObject hitMade = HitObject::MakeHit(
        accelStruct,
        0,  // InstanceIndex
        0,  // GeometryIndex
        0,  // PrimitiveIndex
        HIT_KIND_TRIANGLE_FRONT_FACE,
        0,  // RayContributionToHitGroupIndex
        1,  // MultiplierForGeometryContributionToHitGroupIndex
        ray,
        attribs);

    // 3. MakeHit (explicit index) - creates HitObject with explicit record index
    HitObject hitMadeExplicit = HitObject::MakeHit(
        0,  // HitGroupRecordIndex
        accelStruct,
        0,  // InstanceIndex
        0,  // GeometryIndex
        0,  // PrimitiveIndex
        HIT_KIND_TRIANGLE_FRONT_FACE,
        ray,
        attribs);

    // 4. MakeMiss (NVAPI) - creates HitObject representing a miss (no RayFlags)
    HitObject missObj = HitObject::MakeMiss(
        0,  // MissShaderIndex
        ray);

    // 5. MakeNop - creates HitObject representing no operation
    HitObject nopObj = HitObject::MakeNop();

    // 6. Invoke (NVAPI) - requires AccelerationStructure parameter
    HitObject::Invoke(accelStruct, hitFromTrace, payload);

    outputBuffer[idx] = uint(payload.color.x);
}
