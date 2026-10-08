//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b5a069f423eb5aab235bbf37036927b550b22b305280b48e6cb4e19f40a3c462
//META: purpose=Fires diagnostic E33080 (cannot-have-opaque-member-when-inheriting-dyn-interface) - opaque member with dyn interface inheritance
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33080
//META: catalog_name=cannot-have-opaque-member-when-inheriting-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct inheriting a `dyn` interface and containing an opaque
// (resource) member is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
}

struct S : IFoo
{
    Texture2D<float> tex;
}

//CHECK: E33080
