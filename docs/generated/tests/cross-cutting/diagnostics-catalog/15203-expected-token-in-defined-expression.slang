//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E15203 (expected-token-in-defined-expression) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15203
//META: catalog_name=expected-token-in-defined-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15203.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if defined(
#endif
void main() {}

//CHECK: E15203
