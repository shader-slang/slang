//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#constant-buffers-and-texture-buffers
//META: doc_section_digest=61e21c1be5bb416ba6baceab286e906aa9cc16593b33364af40a28f57a31f79c
//META: purpose=A `tbuffer` declaration uses the same expansion as `cbuffer` but with `TextureBuffer<T>`, producing a `tbuffer` keyword in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `tbuffer` block emits as a `tbuffer` (not `cbuffer`) keyword in HLSL.
// The compute shader reads `texData.x` from `MyTex`; CHECK lines assert the emitted
// HLSL contains the `tbuffer` keyword and the block name, distinguishing it from cbuffer.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

tbuffer MyTex : register(t0)
{
    float4 texData;
}
RWStructuredBuffer<float> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    output[0] = texData.x;
}

// CHECK: tbuffer
// CHECK: MyTex
