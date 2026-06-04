//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E15204 (directive-expects-expression) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15204
//META: catalog_name=directive-expects-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15204.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if 1
#elif
#endif
void main() {}

//CHECK: E15204
