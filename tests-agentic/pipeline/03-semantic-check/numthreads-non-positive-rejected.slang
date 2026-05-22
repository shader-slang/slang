//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=6ed31708085f4823fd353a550243a17316068330fb97dc1ed367bc25d6471443
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
