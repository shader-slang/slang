//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=ab729adc5c4bfaabc81344dfe31170841f8c87ee52d7df58f2988f73f8d33f7f
//META: purpose=Attribute-value checking rejects a non-positive integer in `[numthreads]` (E31102).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// numthreads boundary: the lower edge — zero — is invalid;
// the diagnostic E31102 cites the offending value.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[numthreads(0,1,1)] void main() {}
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '0'
*/
// CHECK: E31102
