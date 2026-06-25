//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Negative: `[numthreads(0,1,1)]` is rejected with diagnostic E31102 ("expected a positive integer in 'numthreads' attribute, got '0'") before Metal lowering ever runs.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: zero-threadgroup-size negative case. The check
// fires in the front end, so the Metal pipeline never sees this
// shader. E31102 is emitted with the literal `0` quoted in the
// message text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(0, 1, 1)]
//CHECK: E31102
//CHECK: numthreads
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}
