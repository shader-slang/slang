//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c77fee7161ce8202ebbab6d6eb6c82335dc0e6c705fca9841cc6d946a470895d
//META: purpose=Fires diagnostic E33073 (cannot-have-associated-type-in-dyn-interface) - dyn interfaces cannot have associatedType members
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33073
//META: catalog_name=cannot-have-associated-type-in-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `dyn` interface containing an associatedtype is rejected
// under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    associatedtype T;
}

//CHECK: E33073
