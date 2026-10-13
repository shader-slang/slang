//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d534012dc94c19fc507bd3e620a17c032ba4ebd1e5bca4714b20f0e56b8ea81b
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

//CHECK: E33074
