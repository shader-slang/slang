//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=52fdb298e55744b84762cd71852daaf6f2817b5ab5bf0a76ad01dbea2015e101
//META: purpose=Fires diagnostic E33078 (cannot-use-extension-to-make-type-conform-to-dyn-interface) - extension cannot conform to dyn interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33078
//META: catalog_name=cannot-use-extension-to-make-type-conform-to-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An extension making a type conform to a `dyn` interface is
// rejected under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    int f();
}

struct S {}

extension S : IFoo
{
    int f() { return 0; }
}

//CHECK: E33078
