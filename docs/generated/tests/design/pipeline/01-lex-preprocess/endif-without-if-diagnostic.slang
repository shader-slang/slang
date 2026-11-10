//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T10:40:15+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=7e4fe030e4c8d273dce73bfc62475650108e02955445a796df82003091653dfa
//META: purpose=A #endif with no open conditional is diagnosed as its own error (DirectiveWithoutIf), separate from the unclosed-conditional diagnostic.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The stray closer is a separate diagnostic from the end-of-file
// "unclosed conditional" one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#endif
/*CHECK:
 ^^^^^ 'endif' directive without '#if'
*/

void main() {}
