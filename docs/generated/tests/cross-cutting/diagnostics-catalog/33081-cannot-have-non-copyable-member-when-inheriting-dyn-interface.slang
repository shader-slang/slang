//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E33081
