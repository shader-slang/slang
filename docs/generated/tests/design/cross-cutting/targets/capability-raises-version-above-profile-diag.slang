//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles-versus-explicit--capability
//META: doc_section_digest=aa5b75d19e741d5bc36e7de273e6e9a576a676ed130d76b894b28a5c65ebc8e9
//META: purpose=A `-capability` whose atom inherits a higher version than the pinned `-profile` in the same family is reported as E00046 rather than silently raising the emitted target version.
//META: intent=negative
//META: pipeline_stage=cli-options
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Many capability atoms inherit from a version atom, so an added `-capability`
// can silently raise the emitted target version above what `-profile` asked
// for. The option parser refuses to let the two options disagree quietly:
// `doRequestedCapabilitiesRaiseTargetVersionAboveProfile` folds each requested
// atom into the profile's capability set one at a time and, if the resulting
// highest version in the family exceeds the pinned one, diagnoses it.
//
// `SPV_KHR_cooperative_matrix` inherits `_spirv_1_6`, so pinning `spirv_1_3`
// and requesting that atom is the documented conflict. The diagnostic is a
// command-line error with no source location, so it is matched by code rather
// than by a caret annotation.
//
// The accepting side of the same line — `-profile spirv_1_6`, no `-profile` at
// all, and a cross-family profile — is covered by
// `capability-version-raise-accepted-emission.slang`. Only the pair shows that
// the check is about the version family rather than about the atom.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -profile spirv_1_3 -capability SPV_KHR_cooperative_matrix -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = float(tid.x);
}

//CHECK: E00046
