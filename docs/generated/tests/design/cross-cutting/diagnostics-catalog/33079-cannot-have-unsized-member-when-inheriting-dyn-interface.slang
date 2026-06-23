//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E33079 (cannot-have-unsized-member-when-inheriting-dyn-interface) - unsized member with dyn interface inheritance
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33079
//META: catalog_name=cannot-have-unsized-member-when-inheriting-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct inheriting a `dyn` interface and containing an unsized
// member is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
}

struct S : IFoo
{
    int v[];
}

// CHECK: E33079
