//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=316c4a1cd15b4a38389e1bb05e0cc6ef67e91289db66f38a96d2b6c1f68e733f
//META: purpose=Fires diagnostic E39010 (expected-space-index) - expected a register space index after 'space'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39010
//META: catalog_name=expected-space-index
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `space` without a numeric index triggers E39010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main -profile sm_5_0

Texture2D<float> tex : register(t0, space);

[numthreads(1,1,1)]
void main() {}

//CHECK: E39010
