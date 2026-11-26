//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/compiler-core/slang-lexer-diagnostic-defs.h
//META: doc_section_digest=f5e45e53f82837da69e27ec2cab80c5e1444c6de61bad520e51f53ee597b64d1
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