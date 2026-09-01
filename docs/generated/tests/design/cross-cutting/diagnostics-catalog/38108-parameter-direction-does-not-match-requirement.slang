//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ec77fcc11589094fbe966dab61d516e40f9bc33bc3152a89700795990ec62978
//META: purpose=Fires diagnostic E38108 (parameter-direction-does-not-match-requirement) - parameter direction mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38108
//META: catalog_name=parameter-direction-does-not-match-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An implementer's parameter direction differs from the
// interface requirement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    void f(out int x);
}

struct S : IFoo
{
    void f(int x) {}
}

//CHECK: E38108
