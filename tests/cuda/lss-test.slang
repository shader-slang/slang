//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK_: __global__ void __closesthit__closestHitShaderLss
//CHECK: optixGetSpherePositionAndRadius
//CHECK: optixGetLssPositionsAndRadii
//CHECK: optixIsSphereHit
//CHECK: optixIsLSSHit

//CHECK: optixHitObjectGetSpherePositionAndRadius
//CHECK: optixHitObjectGetLssPositionsAndRadii
//CHECK: optixHitObjectIsSphereHit
//CHECK: optixHitObjectIsLSSHit

//CHECK_: __global__ void __raygen__raygenShaderLSS()
//CHECK: optixHitObjectGetSpherePositionAndRadius
//CHECK: optixHitObjectGetLssPositionsAndRadii
//CHECK: optixHitObjectIsSphereHit
//CHECK: optixHitObjectIsLSSHit

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
    
    // Test HitObject API functions
    HitObject hitObj;
    float4 sphereData = hitObj.GetSpherePositionAndRadius();
    float2x4 lssData = hitObj.GetLssPositionsAndRadii();
    bool isSphereHit = hitObj.IsSphereHit();
    bool isLssHit = hitObj.IsLssHit();
}

[shader("raygen")]
void raygenShaderLSS(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Test HitObject API functions
    HitObject hitObj;
    float4 sphereData = hitObj.GetSpherePositionAndRadius();
    float2x4 lssData = hitObj.GetLssPositionsAndRadii();
    bool isSphereHit = hitObj.IsSphereHit();
    bool isLssHit = hitObj.IsLssHit();
}