//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=20011fff86e9941dc4570cba9bfbc1dbbb332c8449d092936cad039d082e0312
//META: purpose=Fires diagnostic E33072 (cannot-have-generic-dyn-interface) - dyn interfaces cannot be generic
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33072
//META: catalog_name=cannot-have-generic-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic `dyn` interface is rejected under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo<T>
{
    static const int m;
}

//CHECK: E33072
