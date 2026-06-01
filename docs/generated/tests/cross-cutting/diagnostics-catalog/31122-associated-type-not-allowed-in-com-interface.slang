//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31122 (associated-type-not-allowed-in-com-interface) -- associatedtype not allowed in [COM] interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31122
//META: catalog_name=associated-type-not-allowed-in-com-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[COM("11111111-1111-1111-1111-111111111111")]
interface IFoo {
    associatedtype A;
}


// CHECK: E31122
