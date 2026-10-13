//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=13accb39923c30209ed51862ab1659539875aa6ee6440c6f09a232d3ea4232d3
//META: purpose=Fires diagnostic E36116 (capability-has-multiple-stages) - capability targets multiple stages
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36116
//META: catalog_name=capability-has-multiple-stages
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a `[shader("...")]` value that resolves to a multi-stage
// capability triggers E36116.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -allow-glsl -profile sm_5_0

[shader("compute_fragment")]
void main()
{
}

//CHECK: E36116
