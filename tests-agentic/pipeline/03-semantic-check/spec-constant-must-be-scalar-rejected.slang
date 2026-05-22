//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
//META: purpose=Modifier validation rejects a `[SpecializationConstant]` on a non-scalar/non-enum value (E31218).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Modifier-validation boundary: specialization constants must be
// scalar (or enum). A vector value is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[SpecializationConstant] const int2 X = int2(1,2);
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^^^ specialization constant type error
 ^^^^^^^^^^^^^^^^^^^^^^ specialization constant must be a scalar or enum type.
*/
// CHECK: E31218

[shader("compute")][numthreads(1,1,1)]
void main() {}
