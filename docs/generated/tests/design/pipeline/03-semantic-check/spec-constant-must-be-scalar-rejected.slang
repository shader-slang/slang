//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
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
