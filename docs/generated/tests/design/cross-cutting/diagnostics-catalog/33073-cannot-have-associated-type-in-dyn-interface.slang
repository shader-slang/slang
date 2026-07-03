//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E33073
