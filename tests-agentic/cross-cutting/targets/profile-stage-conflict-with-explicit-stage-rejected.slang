//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=A Profile carries a Stage; `-profile cs_6_5` binds Compute. Asking for `-stage vertex` on the same command line populates the entry point's stage with two different atoms and produces `E00031: conflicting stages`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stage-keyhole conflict at the entry-point input. The doc's Profiles
// section says "A Profile carries: A Stage ...". `cs_6_5` binds the
// Compute stage; the parallel `-stage vertex` argument binds Vertex.
// These populate the same conceptual stage selector with different
// values, and the front-end rejects with E00031 at command-line
// parse.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile cs_6_5 -entry main -stage vertex
//CHECK: conflicting stages have been specified for entry point 'main'

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}
