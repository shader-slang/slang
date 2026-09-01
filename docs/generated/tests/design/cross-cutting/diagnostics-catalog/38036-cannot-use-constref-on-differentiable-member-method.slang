//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bf9709a87d03505b21d57a281f2d43132ddc810d48aef711c415b54b39b39ee8
//META: purpose=Fires diagnostic E38036 (cannot-use-constref-on-differentiable-member-method) - '[constref]' on differentiable member method
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38036
//META: catalog_name=cannot-use-constref-on-differentiable-member-method
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[constref]` on a `[Differentiable]` member method of a
// differentiable type triggers E38036.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S : IDifferentiable
{
    [Differentiable]
    [constref] float compute(float x) { return 0; }
}

//CHECK: E38036
