// conflicting-capability-profile.slang
//
// Test for issue #12099: an explicit `-capability` that cannot be realized at the concrete
// target version an explicit `-profile` selected must be rejected at the command line, rather
// than silently raising the emitted target version. Covers every target-version family a profile
// can pin: SPIR-V (Cases 1-4), and HLSL shader model (Case 5). (Metal is handled by the same code
// path but has no local toolchain to exercise end-to-end here.)
//
// `SPV_KHR_cooperative_matrix` requires SPIR-V 1.6 (capdef:
// `SPV_KHR_cooperative_matrix : _spirv_1_6 + ...`), so it genuinely conflicts with
// `-profile spirv_1_4`. The check keys off the version the compiler would emit rather than a
// hard-coded per-capability requirement, so it needs no update if a capability's requirement ever
// changes. The check is order-independent and also covers the appended
// `-profile <profile>+<capability>` form.
//
// The entry point is a trivial compute shader that requires nothing beyond the profile, so the
// emitted target version is driven solely by the `-profile`/`-capability` pair under test. (Note:
// an entry point's own `[require(...)]` capabilities do NOT feed the emitted target-version
// selection — that is decided from the target caps in `TargetRequest::getTargetCaps()`, i.e. the
// target format plus the explicit `-profile`/`-capability`. So a body using a high-version feature
// would not by itself raise the emitted version and is unnecessary here.)

// Case 1: `-profile` then `-capability`.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//CHECK1: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 2: `-capability` before `-profile` (order-independent).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target spirv-asm -emit-spirv-directly -capability SPV_KHR_cooperative_matrix -profile spirv_1_4 -entry main -stage compute
//CHECK2: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 3: appended `-profile <profile>+<capability>` form.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3):-target spirv-asm -emit-spirv-directly -profile spirv_1_4+SPV_KHR_cooperative_matrix -entry main -stage compute
//CHECK3: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 4: a GLSL-facing alias whose SPIR-V realization requires a higher version. Its GLSL
// realization carries no SPIR-V version atom, but the target-scoped join folds only its SPIR-V
// realization (`spvCooperativeVectorNV`, which needs 1.6) into the SPIR-V target, so it must
// conflict too rather than the GLSL side silently masking the requirement.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK4):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability GL_NV_cooperative_vector -entry main -stage compute
//CHECK4: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Case 5: the same conflict on the HLSL shader-model family. `atomic64`'s HLSL realization requires
// SM 6.6 (capdef: `atomic64 = ... | _sm_6_6 | ...`), so it conflicts with the DX-family profile
// `sm_6_0` on a D3D target. This exercises the shader-model version family, not just SPIR-V/Metal.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK5):-target hlsl -profile sm_6_0 -capability atomic64 -entry main -stage compute
//CHECK5: requires a higher target version than the explicitly requested profile 'sm_6_0'

// Case 6: a target-incompatible `-capability` given alongside a conflicting one must not mask the
// conflict. Here `hlsl` is incompatible with a SPIR-V target; `getTargetCaps()` folds each
// `-capability` individually and skips the incompatible one, so `SPV_KHR_cooperative_matrix` still
// raises the version to 1.6 and must still conflict with `-profile spirv_1_4`. (Regression guard:
// pre-joining all `-capability` atoms into one set before the check let the incompatible `hlsl`
// atom invalidate the combined set and silently drop the conflict.)
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK6):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability SPV_KHR_cooperative_matrix -capability hlsl -entry main -stage compute
//CHECK6: requires a higher target version than the explicitly requested profile 'spirv_1_4'

// Control 1: the same capability at a profile that provides a high enough version is NOT a conflict.
// `SPV_KHR_cooperative_matrix` requires SPIR-V 1.6, and `-profile spirv_1_6` selects exactly 1.6, so
// this must compile with no conflict diagnostic and emit a 1.6 module.
//TEST:SIMPLE(filecheck=OK):-target spirv-asm -emit-spirv-directly -profile spirv_1_6 -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//OK-NOT: requires a higher target version
//OK: ; Version: 1.6

// Control 2: a capability that is satisfiable at the requested version must not be rejected.
// `SPV_KHR_ray_tracing` requires exactly SPIR-V 1.4 (capdef: `SPV_KHR_ray_tracing : _spirv_1_4`),
// so pairing it with `-profile spirv_1_4` is on the boundary and must still compile at 1.4.
//TEST:SIMPLE(filecheck=OK2):-target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability SPV_KHR_ray_tracing -entry main -stage compute
//OK2-NOT: requires a higher target version
//OK2: ; Version: 1.4

// Control 3: the `-profile <glsl-profile>+spirv_1_N` idiom is how users request a concrete SPIR-V
// output version while selecting GLSL-family semantics. `glsl_450` is a GLSL-family profile — it
// does not pin a SPIR-V output version — so the appended `spirv_1_4` is the user *choosing* 1.4,
// not a conflict. This must compile at 1.4. (Regression guard: an earlier version keyed the pin off
// any SPIR-V version atom in the profile caps, misreading glsl_450's implied 1.3 floor as a pin and
// wrongly rejecting this common, valid combination — which broke pre-existing tests such as
// tests/spirv/spirv-version-option.slang and tests/pipeline/rasterization/mesh/hello.slang.)
//TEST:SIMPLE(filecheck=OK3):-target spirv-asm -emit-spirv-directly -profile glsl_450+spirv_1_4 -entry main -stage compute
//OK3-NOT: requires a higher target version
//OK3: ; Version: 1.4

// Control 4: a profile that pins no target version of the target's family (`sm_6_0` is a DX-family
// profile) must not trigger the check, even with a capability that requires a high SPIR-V version.
//TEST:SIMPLE(filecheck=OK4):-target spirv-asm -emit-spirv-directly -profile sm_6_0 -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//OK4-NOT: requires a higher target version

// Control 5: a GLSL text target does not emit a SPIR-V version header, so a SPIR-V-family profile
// pins nothing there — even a capability that requires a high SPIR-V version must not be flagged.
// (The check gates on `isSPIRV`, not `isKhronosTarget`, so the GLSL target is excluded.)
//TEST:SIMPLE(filecheck=OK5):-target glsl -profile spirv_1_4 -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//OK5-NOT: requires a higher target version

// Control 6: the HLSL shader-model counterpart of Control 1 — a high-enough `sm` profile makes the
// SM-gated capability satisfiable, so it must not be flagged. `atomic64` needs SM 6.6, and
// `-profile sm_6_6` pins exactly that.
//TEST:SIMPLE(filecheck=OK6):-target hlsl -profile sm_6_6 -capability atomic64 -entry main -stage compute
//OK6-NOT: requires a higher target version

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}
