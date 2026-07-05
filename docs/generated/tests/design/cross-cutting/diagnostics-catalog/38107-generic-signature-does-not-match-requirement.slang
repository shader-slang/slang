//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38107 (generic-signature-does-not-match-requirement) - generic signature mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38107
//META: catalog_name=generic-signature-does-not-match-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The implementer's generic signature differs from the
// interface requirement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    T f<T : IArithmetic>(T v);
}

struct S : IFoo
{
    T f<T : IComparable>(T v) { return v; }
}

// CHECK: E38107
