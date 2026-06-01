//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38034
