//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E36106 (expect-capability) - expect a capability name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36106
//META: catalog_name=expect-capability
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `[require(...)]` whose argument is a number rather than a
// capability name triggers E36106.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[require(42)]
void helper() {}

// CHECK: E36106
