//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30832 (invalid-type-for-inheritance) -- type '~type:Type' cannot be used for inheritance
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30832
//META: catalog_name=invalid-type-for-inheritance
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S<T> : T {}


// CHECK: E30832
