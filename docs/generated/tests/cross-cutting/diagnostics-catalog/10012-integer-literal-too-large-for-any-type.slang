//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E10012 (integer-literal-too-large-for-any-type) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=lex
//META: catalog_code=10012
//META: catalog_name=integer-literal-too-large-for-any-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E10012.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x = 99999999999999999999999999;
void main() { x; }

//CHECK: E10012
