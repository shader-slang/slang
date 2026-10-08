//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b598c389c744341edcf9ac14c654e737d3b17a5f8103575051fe41812d86872b
//META: purpose=Fires diagnostic E38031 (invalid-use-of-no-diff) - invalid 'no_diff' usage
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38031
//META: catalog_name=invalid-use-of-no-diff
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `no_diff` to a non-call, non-subscript expression
// triggers E38031.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
float f(float x)
{
    return no_diff x;
}

//CHECK: E38031
