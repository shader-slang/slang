//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=f6d63bedcd6d31bb43fb73300201d5b6af73fbcaf7464a55660a5559a1828ae0
//META: purpose=A `#if` directive with no matching `#endif` is reported as a preprocessor diagnostic.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the preprocessor reports the unbalanced `#if` /
// `#endif` failure mode. The opening `#if 1` below has no matching
// `#endif`; the preprocessor must emit a diagnostic at end-of-file
// referring to the open conditional.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#if 1
/*CHECK:
 ^^ see 'if' directive
*/
void main() {}
//CHECK: end of file encountered during preprocessor conditional
// CHECK: E15000
