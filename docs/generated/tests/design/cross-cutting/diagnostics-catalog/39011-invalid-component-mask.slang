//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a6d3ebada4c98172403bd88f0631f5d5fa906666eb415d395237e0321a666934
//META: purpose=Fires diagnostic E39011 (invalid-component-mask) - invalid register component mask
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39011
//META: catalog_name=invalid-component-mask
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `packoffset(c0.q)` uses an invalid component letter and
// triggers E39011.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

cbuffer Cb
{
    float v : packoffset(c0.q);
}

[numthreads(1,1,1)]
void main() {}

//CHECK: E39011
