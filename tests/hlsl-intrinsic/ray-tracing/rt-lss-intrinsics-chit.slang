//TEST:SIMPLE(filecheck=HLSL): -target hlsl -capability hlsl_nvapi
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -capability spvShaderInvocationReorderNV
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NO_NV): -target spirv-asm
//DIAGNOSTIC_TEST:SIMPLE(filecheck=EXT_ONLY): -target spirv-asm -capability spvShaderInvocationReorderEXT -restrictive-capability-check
//HLSL: NvRtSphereObjectPositionAndRadius
//HLSL: NvRtLssObjectPositionsAndRadii
//HLSL: NvRtIsSphereHit
//HLSL: NvRtIsLssHit

//NO_NV: error[E41011]: __target_switch has no compatible target
//NO_NV: error[E41011]: __target_switch has no compatible target
//NO_NV: error[E41011]: __target_switch has no compatible target
//NO_NV: error[E41011]: __target_switch has no compatible target
//EXT_ONLY: error[E41013]: entry point uses capabilities not in specified profile
//EXT_ONLY: Missing capabilities are: 'spvShaderInvocationReorderNV'
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
