//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E33074 (cannot-have-generic-method-in-dyn-interface) - dyn interfaces cannot have generic methods
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33074
//META: catalog_name=cannot-have-generic-method-in-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `dyn` interface containing a generic method is rejected
// under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    T f<T : IArithmetic>(T v);
}

// CHECK: E33074
