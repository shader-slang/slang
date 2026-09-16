//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
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
//CHECK:                ^ newline in literal
;
