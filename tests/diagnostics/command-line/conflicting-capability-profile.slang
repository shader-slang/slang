// conflicting-capability-profile.slang
//
// Test for issue #12099: an explicit `-capability` that cannot be realized at the concrete
// target version an explicit `-profile` selected must be rejected at the command line, rather
// than silently raising the emitted SPIR-V version.
//
// `spvShaderInvocationReorderNV` has only a SPIR-V 1.5 realization today
// (`SPV_EXT_shader_invocation_reorder : _spirv_1_5 + SPV_KHR_ray_tracing`), so it genuinely
// conflicts with `-profile spirv_1_4`. The check is order-independent and also covers the
// appended `-profile <profile>+<capability>` form.

// Case 1: `-profile` then `-capability`.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//CHECK1: the capability 'spvShaderInvocationReorderNV' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 2: `-capability` before `-profile` (order-independent).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderNV -profile spirv_1_4 -entry main -stage raygeneration
//CHECK2: the capability 'spvShaderInvocationReorderNV' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 3: appended `-profile <profile>+<capability>` form.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3):-target spirv-asm -emit-spirv-directly -profile spirv_1_4+spvShaderInvocationReorderNV -entry main -stage raygeneration
//CHECK3: the capability 'spvShaderInvocationReorderNV' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 4: the GLSL-facing alias of the same capability. Its GLSL realization carries no SPIR-V
// version atom, but folding it into a SPIR-V target still forces SPIR-V 1.5 (its SPIR-V realization
// is `spvShaderInvocationReorderNV`), so it must conflict too rather than silently emitting 1.5.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK4):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability GL_NV_shader_invocation_reorder -entry main -stage raygeneration
//CHECK4: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Control 1: the same capability at a profile that provides a high enough version is NOT a conflict.
// SER requires SPIR-V 1.5, and `-profile spirv_1_5` selects exactly 1.5, so this must compile
// with no conflict diagnostic. (Slang canonicalizes the NV request onto the EXT capability, so
// only assert the shared substring rather than the NV/EXT spelling.)
//TEST:SIMPLE(filecheck=OK):-target spirv-asm -emit-spirv-directly -profile spirv_1_5 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//OK-NOT: requires a higher target version
//OK: OpCapability ShaderInvocationReorder

// Control 2: a capability that is satisfiable at the requested version must not be rejected.
// `SPV_KHR_ray_tracing` requires exactly SPIR-V 1.4 (capdef: `SPV_KHR_ray_tracing : _spirv_1_4`),
// so pairing it with `-profile spirv_1_4` is on the boundary and must still compile at 1.4.
//TEST:SIMPLE(filecheck=OK2):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability SPV_KHR_ray_tracing -entry main -stage raygeneration
//OK2-NOT: requires a higher target version
//OK2: ; Version: 1.4

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

struct Payload
{
    uint foo;
};

struct Attributes
{
    float2 barycentrics;
};

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(1.0);
    ray.TMin = 0.0;
    ray.TMax = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);
    if (hitObj.IsHit())
    {
        Attributes attributes = hitObj.GetAttributes<Attributes>();
        outImage[DispatchRaysIndex().xy] = 1.0f + attributes.barycentrics.x;
    }
}
