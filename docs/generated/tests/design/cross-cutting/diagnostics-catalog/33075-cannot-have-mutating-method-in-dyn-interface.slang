//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=84ca250ae0e4dbf0b76d457ec8c87dfea713b66a69f0696f40c916c09d19d075
//META: purpose=Fires diagnostic E33075 (cannot-have-mutating-method-in-dyn-interface) - dyn interfaces cannot have [mutating] methods
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33075
//META: catalog_name=cannot-have-mutating-method-in-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `dyn` interface containing a [mutating] method is rejected
// under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    [mutating]
    void mut(int v);
}

//CHECK: E33075
