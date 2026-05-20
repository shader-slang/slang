//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: divide-by-zero in a preprocessor `#if` expression renders E15202, distinct from the semantic-stage E30002.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `divide-by-zero-in-preprocessor-expression` (15202) to its
// rendered code. This is distinct from the check-stage divide
// (30002) — the two carry separate ids in separate ranges per
// the doc's "unique integer id" claim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#if 1/0
/*CHECK:
     ^ E15202
     ^ division by zero in preprocessor expression
*/
#endif
void main() {}
