//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b7f362a156d5667189e07898b68e1621030f7de6635b4ad056d730039cf90716
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


// CHECK: E31101
