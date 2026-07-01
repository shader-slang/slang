//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=6ed31708085f4823fd353a550243a17316068330fb97dc1ed367bc25d6471443
//META: purpose=Attribute argument checking rejects a non-string-literal argument to `[shader(...)]` (E33071).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Attribute-argument boundary: `[shader(...)]` takes a string
// literal; passing an integer is rejected with E33071.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[shader(123)] void m() {}
//CHECK:^^^ expected a string literal
// CHECK: E33071

void main() {}
