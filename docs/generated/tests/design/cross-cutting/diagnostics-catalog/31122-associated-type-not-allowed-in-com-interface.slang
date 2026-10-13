//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=339b23c499679e8fc1f4e58952b958b59dff6ac9e7cab05800114bbb9a3c0256
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


//CHECK: E31122
