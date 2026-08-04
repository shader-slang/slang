//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38204 (cannot-use-resource-type-in-structured-buffer) - resource type in StructuredBuffer
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=38204
//META: catalog_name=cannot-use-resource-type-in-structured-buffer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Putting a Texture2D (resource) as the element type of a
// StructuredBuffer is rejected at IR validation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

StructuredBuffer<Texture2D<float>> sb;

[numthreads(1,1,1)]
void main()
{
}

// CHECK: E38204
