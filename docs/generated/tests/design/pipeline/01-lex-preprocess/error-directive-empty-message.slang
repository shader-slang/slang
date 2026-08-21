//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Boundary probe of `#error`: a bare `#error` with no message text still fires the documented preprocessor error.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#error` with the empty message — the lower edge of the message
// axis. The diagnostic must still be produced; its formatted text
// just has nothing after `#error:`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#error
/*CHECK:
 ^^^^^ preprocessor error
 ^^^^^ #error:
*/
// CHECK: E15900

void main() {}
