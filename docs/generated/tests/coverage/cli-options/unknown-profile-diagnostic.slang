//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=469973558ae9131458179def57cec815ce506922c2ef82e0e161360ad76f32c5
//META: purpose=Verifies an unrecognized -profile operand is rejected with E00014 naming the unknown profile.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of -profile operand handling (Profile::lookUp returning
// SLANG_PROFILE_UNKNOWN) in slang-options.cpp. An unknown profile name diagnoses
// E00014. The diagnostic is anchored to the synthetic command line, so it is
// pinned by error code. Observed message recorded verbatim: "unknown profile
// 'zzz'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile zzz -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

//CHECK: E00014
