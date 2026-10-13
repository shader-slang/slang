//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2570cdeda6d548ec2acb6b580ab63208f252a2dc5ae6a6ce92aa56741049bce0
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

//CHECK: E33079
