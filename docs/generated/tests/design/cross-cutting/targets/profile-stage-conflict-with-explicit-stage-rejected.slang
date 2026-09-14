//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
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
// CHECK: E00031

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}
