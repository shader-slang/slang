//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8b88e8f8ac28e0c3b18aca9ef7f8503a264a7366ae1aba453fe4eb8598dc4ac3
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

//CHECK: E39009
