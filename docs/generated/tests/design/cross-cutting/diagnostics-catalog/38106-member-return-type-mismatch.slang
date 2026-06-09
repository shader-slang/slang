//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38106 (member-return-type-mismatch) - return type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38106
//META: catalog_name=member-return-type-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An implementer's method has a different return type than the
// interface requires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int f();
}

struct S : IFoo
{
    float f() { return 0; }
}

// CHECK: E38106
