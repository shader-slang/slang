//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T21:16:29Z
//META: source_commit=a621b651a447846e5c57c2678ffbdf87971b8b0c
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30055 (use-of-non-short-circuiting-operator-in-diff-func) - non-short-circuiting `?:` not allowed in differentiable function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30055
//META: catalog_name=use-of-non-short-circuiting-operator-in-diff-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Non-short-circuiting `?:` with a vector condition inside a differentiable function triggers the diagnostic (a scalar condition short-circuits and does not).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
float f(float x, bool2 b)
{
    return (b ? float2(x) : float2(0.0)).x;
}

void main() {}

// CHECK: 30055
