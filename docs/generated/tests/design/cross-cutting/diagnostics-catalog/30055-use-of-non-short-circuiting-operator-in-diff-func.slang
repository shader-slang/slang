//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2c6b49777ee83918e9c045475349590db539546e849f853d84f6a5f00bd994ae
//META: purpose=Fires diagnostic E30055 (use-of-non-short-circuiting-operator-in-diff-func) - non-short-circuiting `?:` not allowed in differentiable function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30055
//META: catalog_name=use-of-non-short-circuiting-operator-in-diff-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-short-circuiting `?:` with a *vector* condition inside a differentiable
// function triggers E30055. The diagnostic is emitted only on the vector-condition
// path of `SemanticsExprVisitor::visitSelectExpr`; a scalar (`bool`) condition
// early-returns with no diagnostic, so the example must use a `bool`N condition.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
float3 f(float3 x, bool3 b)
{
    return b ? x : float3(0.0);
}

void main() {}

//CHECK: 30055
