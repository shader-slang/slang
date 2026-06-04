//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38036
