//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=ae4aa1fa234c5a91862002d52c8aeecec1baa813ca385b39da9d91ae3c677ffe
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
