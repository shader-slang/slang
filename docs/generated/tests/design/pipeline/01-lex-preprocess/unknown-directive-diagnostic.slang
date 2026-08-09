//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=b47fb1d2e5530bf02a1606e2006144db0e35da148f564b5370d15c8fe13ad12f
//META: purpose=An unknown preprocessor directive emits a diagnostic through the DiagnosticSink.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the preprocessor reports an "unknown directive"
// diagnostic for `#`-introduced names that are not in the directive
// callback table.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#not_a_real_directive
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^ unknown directive
 ^^^^^^^^^^^^^^^^^^^^ unknown preprocessor directive 'not_a_real_directive'
*/
// CHECK: E15101

void main() {}
