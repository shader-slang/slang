//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=478e47717b8bd99025cf8b497bcf330364742e44904e1e2c2a4d1a9623483303
//META: purpose=Fires diagnostic E38033 (cannot-use-no-diff-in-non-differentiable-func) - 'no_diff' in non-differentiable function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38033
//META: catalog_name=cannot-use-no-diff-in-non-differentiable-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `no_diff` inside a function that is not marked
// `[Differentiable]` triggers E38033.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

float helper(float x) { return x; }

float f(float x)
{
    return no_diff helper(x);
}

//CHECK: E38033
