//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Negative - [numthreads(-1,1,1)] is rejected with E31102, confirming that the positive-integer validator also catches signed underflow.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative - signed-integer wrap of the leading dimension to -1
// fails the positive-integer check; the same diagnostic E31102
// fires before the numThreads decoration is created.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[numthreads(-1, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '-1'
*/
// CHECK: E31102
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}
