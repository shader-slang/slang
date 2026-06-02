//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#constant-buffers-and-texture-buffers
//META: doc_section_digest=61e21c1be5bb416ba6baceab286e906aa9cc16593b33364af40a28f57a31f79c
//META: purpose=A `cbuffer` declaration expands to an anonymous struct in a `ConstantBuffer<>`, and fields are accessible directly by name in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `cbuffer` block emits as a named HLSL cbuffer with its fields accessible.
// The compute shader reads `count` from `MyParams`; CHECK asserts the emitted HLSL contains
// the cbuffer keyword, the block name, and the field name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

cbuffer MyParams : register(b0)
{
    float4 color;
    int count;
}
RWStructuredBuffer<int> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    output[0] = count;
}

// CHECK: cbuffer
// CHECK: MyParams
// CHECK: count
