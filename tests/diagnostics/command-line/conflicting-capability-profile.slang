// conflicting-capability-profile.slang
//
// Test for issue #12099: an explicit `-capability` that cannot be realized at the concrete
// target version an explicit `-profile` selected must be rejected at the command line, rather
// than silently raising the emitted SPIR-V version.
//
// `SPV_KHR_cooperative_matrix` requires SPIR-V 1.6 (capdef:
// `SPV_KHR_cooperative_matrix : _spirv_1_6 + ...`), so it genuinely conflicts with
// `-profile spirv_1_4`. The check keys off the version the compiler would emit rather than a
// hard-coded per-capability requirement, so it needs no update if a capability's requirement ever
// changes. The check is order-independent and also covers the appended
// `-profile <profile>+<capability>` form.

// Case 1: `-profile` then `-capability`.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability SPV_KHR_cooperative_matrix -entry main -stage raygeneration
//CHECK1: the capability 'SPV_KHR_cooperative_matrix' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 2: `-capability` before `-profile` (order-independent).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target spirv-asm -emit-spirv-directly -capability SPV_KHR_cooperative_matrix -profile spirv_1_4 -entry main -stage raygeneration
//CHECK2: the capability 'SPV_KHR_cooperative_matrix' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 3: appended `-profile <profile>+<capability>` form.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3):-target spirv-asm -emit-spirv-directly -profile spirv_1_4+SPV_KHR_cooperative_matrix -entry main -stage raygeneration
//CHECK3: the capability 'SPV_KHR_cooperative_matrix' requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 4: a GLSL-facing alias whose SPIR-V realization requires a higher version. Its GLSL
// realization carries no SPIR-V version atom, but folding it into a SPIR-V target still forces
// SPIR-V 1.6 (its SPIR-V side is `spvCooperativeVectorNV`, which needs 1.6), so it must conflict
// too rather than the GLSL side silently masking the requirement.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK4):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability GL_NV_cooperative_vector -entry main -stage raygeneration
//CHECK4: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Control 1: the same capability at a profile that provides a high enough version is NOT a conflict.
// `SPV_KHR_cooperative_matrix` requires SPIR-V 1.6, and `-profile spirv_1_6` selects exactly 1.6, so
// this must compile with no conflict diagnostic and emit a 1.6 module.
//TEST:SIMPLE(filecheck=OK):-target spirv-asm -emit-spirv-directly -profile spirv_1_6 -capability SPV_KHR_cooperative_matrix -entry main -stage raygeneration
//OK-NOT: requires a higher target version
//OK: ; Version: 1.6

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
