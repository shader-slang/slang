//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Negative probe: a raw newline inside a string literal is diagnosed at scan time as a newline in literal (E10005).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the scan-time check that a string literal must close on
// the physical line it opened: the literal below is cut off by the
// end of its line, so the scanner reports newline-in-literal,
// pinned to code E10005. The literal still becomes a token ending
// at the newline, so no cascading diagnostics follow.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// CHECK: E10005
// CHECK: newline in literal

String s = "unterminated
;
