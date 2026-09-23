//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=540ef17974314fc5922c3ff229ec452bfd4c09fbba1b5b227ca3eed6c76882fc
//META: purpose=Fires diagnostic E33082 (cannot-conform-generic-to-dyn-interface) - generic type cannot conform to dyn interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33082
//META: catalog_name=cannot-conform-generic-to-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic type conforming to a `dyn` interface is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    int f();
}

struct G<T : IArithmetic> : IFoo
{
    T v;
    int f() { return 0; }
}

//CHECK: E33082
