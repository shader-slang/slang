//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0195bc3e005617bc1235e6b481351c5c3cd7f28a3b8b71821b78820569c2cd0a
//META: purpose=Fires diagnostic E30015 (undefined-identifier) - undefined identifier
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30015
//META: catalog_name=undefined-identifier
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Name lookup fails when no declaration is visible for an identifier. The CHECK
// pins the undefined-identifier error, which is the checker's report for an
// empty lookup result.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = notDefinedAnywhereXyz; }

//CHECK: E30015
