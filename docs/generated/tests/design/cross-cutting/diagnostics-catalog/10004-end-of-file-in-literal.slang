//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=f004998f3bf790529ba3e2db6c6b480908b33b40f429b48e302db06256be33a3
//META: purpose=Fires diagnostic E10004 (end-of-file-in-literal) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=10004
//META: catalog_name=endOfFileInLiteral
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10004.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
//CHECK: E10004
int x = "unterminated