//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E39009 (expected-space) - expected 'space'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39009
//META: catalog_name=expected-space
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Providing a misspelled space specifier (`spaze`) triggers E39009.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

Texture2D<float> tex : register(t0, spaze0);

[numthreads(1,1,1)]
void main() {}

// CHECK: E39009
