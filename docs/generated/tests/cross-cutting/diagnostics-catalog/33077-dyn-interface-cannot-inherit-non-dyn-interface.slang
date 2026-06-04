//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E33077
