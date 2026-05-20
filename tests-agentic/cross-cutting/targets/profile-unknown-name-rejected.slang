//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=A `-profile` value that does not appear in slang-profile-defs.h is rejected with `E00014: unknown profile`; profile parsing is closed-set against the Family + Version table.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the Profiles section's closed-set spelling table.
// `bogus_profile` is not declared in the PROFILE() macro list in
// slang-profile-defs.h, so the front-end rejects it at command-line
// parse with E00014. The test deliberately uses a non-`sm_*`
// spelling so it does not duplicate the `cs_99_99` smoke negative
// already covered by target-pipelines/hlsl.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile bogus_profile -entry main -stage compute
//CHECK: unknown profile 'bogus_profile'

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}
