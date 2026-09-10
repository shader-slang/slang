//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3271cd6f3937c0f9be11454aa052473d89e6e0c7ad1e010e820d29be16b86c88
//META: purpose=Fires diagnostic E30047 (argument-expected-lvalue) - argument must be l-value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30047
//META: catalog_name=argument-expected-lvalue
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `out` parameter is written back to its argument, so the argument must be
// an l-value. Passing a literal gives nothing to write back to, which the CHECK
// observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void f(out int x) { x = 1; }
void main() { f(1); }

//CHECK: E30047
