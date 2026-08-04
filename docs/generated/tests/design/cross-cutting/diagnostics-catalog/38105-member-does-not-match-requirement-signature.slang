//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38105 (member-does-not-match-requirement-signature) - interface requirement mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38105
//META: catalog_name=member-does-not-match-requirement-signature
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An implementer's parameter type differs from the interface
// requirement, with no overloads, triggers E38105.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int f(int x);
}

struct S : IFoo
{
    int f(float x) { return 0; }
}

// CHECK: E38105
