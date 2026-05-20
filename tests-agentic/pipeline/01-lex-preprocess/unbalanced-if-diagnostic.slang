//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=7b0494434454c9476d516b02e8b2bb91395544db9dcff77b2783b0b8868b34dc
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
