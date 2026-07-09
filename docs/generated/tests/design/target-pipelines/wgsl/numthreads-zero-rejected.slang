//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=602dd96dd9a5139f2ccd4e3f857107fe47c9705a3ba52eae7e88a90893f9d664
//META: purpose=[numthreads(0,1,1)] is rejected with diagnostic E31102 ('invalid numthreads value') because the @workgroup_size axis cannot be zero
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Below-the-lower-edge of the @workgroup_size axis: a zero
// dimension is rejected at semantic-check time. The caret column
// of the offending token is 2 ('[' is column 1), so we use the
// block-comment CHECK form to attach annotations to that column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(0,1,1)]
/*CHECK:
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '0'
*/
// CHECK: E31102
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}
