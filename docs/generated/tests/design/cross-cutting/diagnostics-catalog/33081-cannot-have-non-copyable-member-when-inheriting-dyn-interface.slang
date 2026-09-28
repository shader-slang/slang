//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=619857406bef4cd5dcd086a3361f70438dbb401a24ec70580c300caa35140bfd
//META: purpose=Fires diagnostic E33081 (cannot-have-non-copyable-member-when-inheriting-dyn-interface) - non-copyable member with dyn interface inheritance
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33081
//META: catalog_name=cannot-have-non-copyable-member-when-inheriting-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct inheriting a `dyn` interface and containing a
// non-copyable member is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
}

[__NonCopyableType]
struct NC
{
    int v;
}

struct S : IFoo
{
    NC v;
}

//CHECK: E33081
