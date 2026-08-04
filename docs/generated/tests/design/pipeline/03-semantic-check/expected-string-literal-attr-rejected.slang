//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=ab729adc5c4bfaabc81344dfe31170841f8c87ee52d7df58f2988f73f8d33f7f
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
