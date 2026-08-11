//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=55be0be723759d14bdac9691506b00196fab7ed1d77a5dc07456e9d89eed162c
//META: purpose=Fires diagnostic E33077 (dyn-interface-cannot-inherit-non-dyn-interface) - dyn interface inheritance error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33077
//META: catalog_name=dyn-interface-cannot-inherit-non-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `dyn` interface inheriting from a non-`dyn` interface is
// rejected under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

interface IBase
{
}

dyn interface IFoo : IBase
{
    int f(int v);
}

//CHECK: E33077
