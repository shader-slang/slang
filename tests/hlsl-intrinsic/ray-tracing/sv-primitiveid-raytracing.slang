// Regression test for #11197: SV_PrimitiveID is allowed in intersection,
// any-hit, and closest-hit stages per the GLSL_EXT_ray_tracing spec, but
// the front-end accessor table for `sv_primitiveid` only listed the
// rasterization stages, and `_processEntryPointVaryingParameter` rejected
// any `in` parameter on intersection / routed it to the hit-attribute
// layout path on any-hit and closest-hit regardless of semantic.
//
// We pin the fix on two emit paths:
//
//   - SPIR-V via direct emit (`-emit-spirv-directly`).
//   - DXIL (`-target dxil-assembly`).
//
// For each ray-tracing stage we verify (a) the parameter reaches the
// right system-value lowering (`BuiltIn PrimitiveId` in SPIR-V,
// `dx.op.primitiveIndex.i32` in DXIL) and (b) for any-hit / closest-hit,
// the parameter is *not* routed through the hit-attribute layout path
// (`OpTypePointer HitAttribute*`) — that proves the bypass in
// `_processEntryPointVaryingParameter` actually fired.
//
// Note: the `-emit-spirv-via-glsl` path is intentionally not covered
// here. With the front-end gate lifted, an `int pid : SV_PrimitiveID`
// parameter currently survives all the way to the GLSL legalize pass
// and ends up declared as a plain `int pid_0;` global rather than
// being mapped to `gl_PrimitiveID` (because `consolidateRayTracingParameters`
// → `handleSingleParam` does not run the SV → gl_* resolution that
// `createGLSLGlobalVaryings` performs for rasterization stages). Fixing
// the GLSL legalize path is a separate change tracked as follow-up on
// #11197.

//TEST:SIMPLE(filecheck=ISECT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_SPIRV):-target spirv-assembly -emit-spirv-directly -entry main_chit -stage closesthit

//TEST:SIMPLE(filecheck=ISECT_DXIL):-target dxil-assembly -profile sm_6_5 -entry main_isect -stage intersection
//TEST:SIMPLE(filecheck=AHIT_DXIL):-target dxil-assembly -profile sm_6_5 -entry main_ahit -stage anyhit
//TEST:SIMPLE(filecheck=CHIT_DXIL):-target dxil-assembly -profile sm_6_5 -entry main_chit -stage closesthit

struct Payload
{
    float val;
};

struct Attribs
{
    float v;
};

[shader("intersection")]
void main_isect(int pid : SV_PrimitiveID)
{
    Attribs a = { float(pid) };
    ReportHit<Attribs>(1.0, 0, a);
}
// Intersection emits a HitAttribute pointer for the `Attribs` argument
// passed to `ReportHit<Attribs>`, so we cannot use `CHECK-NOT` on the
// type pointer here. The bypass is exercised in any-hit / closest-hit
// below, where there is no other input that would justify a HitAttribute
// pointer.
//
// ISECT_SPIRV: BuiltIn PrimitiveId
//
// ISECT_DXIL: call i32 @dx.op.primitiveIndex.i32

[shader("anyhit")]
void main_ahit(inout Payload payload, int pid : SV_PrimitiveID)
{
    payload.val = float(pid);
}
// No `in Attribs` parameter, so the only `in` is `pid` carrying SV_PrimitiveID.
// If the bypass had not fired, `pid` would have been routed through the
// hit-attribute layout path and HitAttribute pointers would appear in the
// SPIR-V output. Assert their absence directly.
//
// AHIT_SPIRV: BuiltIn PrimitiveId
// AHIT_SPIRV-NOT: OpTypePointer HitAttribute
//
// AHIT_DXIL: call i32 @dx.op.primitiveIndex.i32

[shader("closesthit")]
void main_chit(inout Payload payload, int pid : SV_PrimitiveID)
{
    payload.val = float(pid);
}
// CHIT_SPIRV: BuiltIn PrimitiveId
// CHIT_SPIRV-NOT: OpTypePointer HitAttribute
//
// CHIT_DXIL: call i32 @dx.op.primitiveIndex.i32
