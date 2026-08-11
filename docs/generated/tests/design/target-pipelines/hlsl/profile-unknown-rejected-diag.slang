//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Negative: an unknown profile string (cs_99_99) is rejected on the command line with an "unknown profile" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: profile out-of-band — a profile that does not
// exist in the SM table must be rejected before code generation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -profile cs_99_99
//CHECK: unknown profile 'cs_99_99'
// CHECK: E00014

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}
