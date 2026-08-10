//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5fa8c32d1f84cd9cb7e96980a93eea5b978b02a744f82c419e67563ba30dbdc2
//META: purpose=Fires diagnostic E31101 (unknown-image-format-name) -- unknown image format
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31101
//META: catalog_name=unknown-image-format-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[format("not_a_format")]
RWTexture2D<float4> tex;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31101
