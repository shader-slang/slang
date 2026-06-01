//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E20018 (const-not-allowed-on-type) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20018
//META: catalog_name=const-not-allowed-on-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E20018.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct Wrap<T> { T value; }
Wrap<const int> w;
void main() { w.value; }

//CHECK: E20018
