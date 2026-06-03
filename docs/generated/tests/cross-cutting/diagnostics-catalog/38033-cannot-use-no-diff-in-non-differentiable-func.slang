//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E38033
