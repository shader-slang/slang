//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30061 (expected-a-namespace) - expected a namespace
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30061
//META: catalog_name=expected-a-namespace
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `using` directive applied to a non-namespace name triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {}

using S;

void main() {}

// CHECK: E30061
