//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
//META: purpose=Modifier validation rejects an attribute placed on a target where it is not allowed (E31002).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Modifier-validation boundary: `[numthreads]` is only valid on
// entry-point functions; applying it to a struct is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[numthreads(1,1,1)]
/*CHECK:
 ^^^^^^^^^^ invalid attribute placement
 ^^^^^^^^^^ attribute 'numthreads' is not valid here
*/
// CHECK: E31002
struct S {}
[shader("compute")][numthreads(1,1,1)]
void main() {}
