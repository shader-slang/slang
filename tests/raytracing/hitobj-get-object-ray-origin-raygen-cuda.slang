//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenShader

[raypayload]
struct RayPayload
{
    float3 rayOrigin : read(caller) : write(caller, closesthit, miss);
};

uniform RaytracingAccelerationStructure sceneBVH;

struct Result
{
    float3 rayOrigin;
    float3 rayDirection;
};

uniform RWStructuredBuffer<Result> resultBuffer;

[shader("raygeneration")]
void rayGenShader()
{
    RayDesc ray;
    ray.Origin = {0.1, 0.1, 0.1};
    ray.Direction = {0.0, 0.0, 1.0};
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { {} };
    HitObject hitobj = HitObject.TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    // These calls should compile successfully for CUDA target in raygen stage
    resultBuffer[0].rayOrigin = hitobj.GetObjectRayOrigin();
    resultBuffer[0].rayDirection = hitobj.GetObjectRayDirection();
}

// CHECK: optixGetObjectRayOrigin
// CHECK: optixGetObjectRayDirection