//TEST:SIMPLE(filecheck=HLSL): -target hlsl
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm
//HLSL: NvRtSphereObjectPositionAndRadius
//HLSL: NvRtLssObjectPositionsAndRadii
//HLSL: NvRtIsSphereHit
//HLSL: NvRtIsLssHit

//SPIRV: HitSpherePositionNV
//SPIRV: HitSphereRadiusNV
//SPIRV: HitLSSPositionsNV
//SPIRV: HitIsSphereNV
//SPIRV: HitIsLSSNV

// Hit object variants
//SPIRV: OpHitObjectGetSpherePositionNV
//SPIRV: OpHitObjectGetSphereRadiusNV
//SPIRV: OpHitObjectGetLSSPositionsNV
//SPIRV: OpHitObjectGetLSSRadiiNV
//SPIRV: OpHitObjectIsSphereHitNV
//SPIRV: OpHitObjectIsLSSHitNV

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