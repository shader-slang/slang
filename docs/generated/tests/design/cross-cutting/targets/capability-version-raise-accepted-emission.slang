//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles-versus-explicit--capability
//META: doc_section_digest=aa5b75d19e741d5bc36e7de273e6e9a576a676ed130d76b894b28a5c65ebc8e9
//META: purpose=The same capability that conflicts with `-profile spirv_1_3` is accepted when the profile pins a high enough version, when no profile is given, and when the profile names a different version family — each emitting a 1.6 module.
//META: intent=boundary
//META: pipeline_stage=cli-options
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The accepting half of the E00046 pair. `SPV_KHR_cooperative_matrix` inherits
// `_spirv_1_6`, and the conflict check fires only when the profile pins a
// *lower* version *of the same family*. This test pins the three documented
// ways that condition fails to hold:
//
//   - the profile already pins 1.6, so nothing is raised;
//   - no `-profile` is given, so the function returns false because the profile
//     pins no version of that family; and
//   - the profile names a different family entirely (`glsl_450`), so the
//     SPIR-V version is free to come from the capability.
//
// All three emit a 1.6 module, which is what makes the negative case in
// `capability-raises-version-above-profile-diag.slang` meaningful: the atom is
// not rejected on its own merits, only when it contradicts an explicit pin.

//TEST:SIMPLE(filecheck=PINNED):-target spirv-asm -profile spirv_1_6 -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//TEST:SIMPLE(filecheck=NOPROFILE):-target spirv-asm -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//TEST:SIMPLE(filecheck=CROSSFAMILY):-target spirv-asm -profile glsl_450 -capability SPV_KHR_cooperative_matrix -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = float(tid.x);
}

// PINNED: ; Version: 1.6
// NOPROFILE: ; Version: 1.6
// CROSSFAMILY: ; Version: 1.6
