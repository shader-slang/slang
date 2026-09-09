//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=f6d63bedcd6d31bb43fb73300201d5b6af73fbcaf7464a55660a5559a1828ae0
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
