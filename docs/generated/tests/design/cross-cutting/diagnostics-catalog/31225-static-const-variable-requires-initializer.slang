//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E31225 (static-const-variable-requires-initializer) - missing initializer for static const
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31225
//META: catalog_name=static-const-variable-requires-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static const global without an initializer triggers E31225.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const int g;

// CHECK: E31225
