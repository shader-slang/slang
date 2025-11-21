//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry rayGenShader -stage raygeneration
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry rayGenShader -stage raygeneration

//TEST_INPUT: set sceneBVH = AccelerationStructure
uniform RaytracingAccelerationStructure sceneBVH;

[raypayload]
struct RayPayload
{
    float3 rayOrigin : read(caller) : write(caller, closesthit, miss);
};

struct Result
{
    float3 rayOrigin;
    float3 rayDirection;
};

//TEST_INPUT:set resultBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
uniform RWStructuredBuffer<Result> resultBuffer;

[shader("raygeneration")]
void rayGenShader()
{
    RayDesc ray;
    ray.Origin = float3(0.1, 0.1, 0.1);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload = { {} };

    // SPIRV: OpHitObjectTraceRayNV
    // CUDA: optixTraverse
    HitObject hitobj = HitObject::TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    // Test HitObject.GetObjectRayOrigin() and GetObjectRayDirection() in raygen stage
    // These should compile successfully for SPIRV and CUDA targets
    // Note: HLSL does not have these methods on HitObject (no NVAPI support)

    // SPIRV: OpHitObjectGetObjectRayOriginNV
    // CUDA: optixGetObjectRayOrigin
    resultBuffer[0].rayOrigin = hitobj.GetObjectRayOrigin();

    // SPIRV: OpHitObjectGetObjectRayDirectionNV
    // CUDA: optixGetObjectRayDirection
    resultBuffer[0].rayDirection = hitobj.GetObjectRayDirection();
}
