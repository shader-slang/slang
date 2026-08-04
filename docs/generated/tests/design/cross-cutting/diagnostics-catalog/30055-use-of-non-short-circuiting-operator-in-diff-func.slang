//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T07:55:45Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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
