//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E33070 (expected-function) - expected a function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33070
//META: catalog_name=expected-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a non-function name as the patchconstantfunc argument
// triggers E33070.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int g = 0;

[domain("tri")]
[patchconstantfunc("g")]
void main() {}

// CHECK: E33070
