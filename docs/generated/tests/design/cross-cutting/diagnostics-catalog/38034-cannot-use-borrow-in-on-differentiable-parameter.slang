//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3caa2e4a17cb6381a7fe75edc6ee83648514eab8205fc56035c19c0922586d14
//META: purpose=Fires diagnostic E38034 (cannot-use-borrow-in-on-differentiable-parameter) - 'borrow in' on differentiable parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38034
//META: catalog_name=cannot-use-borrow-in-on-differentiable-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__constref` on a differentiable parameter triggers E38034.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
float f(__constref float3 v)
{
    return v.x;
}

//CHECK: E38034
