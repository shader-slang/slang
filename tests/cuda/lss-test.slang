//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK_: __global__ void __closesthit__closestHitShaderLss
//CHECK: optixGetSpherePositionAndRadius
//CHECK: optixObjectPositionsAndRadii
//CHECK: optixIsSphereHit
//CHECK: optixIsLSSHit

struct RayPayload
{
    float4 color;
    float2x4 lssData;
    bool isSphere;
    bool isLss;
};

[shader("closesthit")]
void closestHitShaderLss(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Test TraceRay API functions
    payload.color = GetSpherePositionAndRadius();
    payload.lssData = GetLssPositionsAndRadii();
    payload.isSphere = IsSphereHit();
    payload.isLss = IsLssHit();
    
// TODO: This will be enabled once issue #6647 is completed.
#if 0
    // Test HitObject API functions
    HitObject hitObj;
    float4 sphereData = hitObj.GetSphereObjectPositionAndRadius();
    float2x4 lssData = hitObj.GetLssObjectPositionsAndRadii();
    bool isSphereHit = hitObj.IsSphereHit();
    bool isLssHit = hitObj.IsLssHit();
#endif
}