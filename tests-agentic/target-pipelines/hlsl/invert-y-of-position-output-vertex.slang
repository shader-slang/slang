//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#invertyofpositionoutput
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=The Vulkan-cross-API option flag -fvk-invert-y enables invertYOfPositionOutput; the IR pass negates the y component of the SV_Position output before the writeback in the emitted target text.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Phase C row 16 lists invertYOfPositionOutput gated on
// `isKhronosTarget || HLSL` together with the `VulkanInvertY` option.
// Slang's CLI exposes this as `-fvk-invert-y`. Emission target here is
// GLSL because that's the canonical cross-API porting path; the IR
// pass negates the y component of `SV_Position` before assignment to
// `gl_Position`. We pin a `-` on the y-component assignment in the
// emitted GLSL.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage vertex -profile vs_5_1 -fvk-invert-y

struct VOutput
{
    float4 v : SV_Position;
}

VOutput main()
{
    VOutput output;
    output.v = float4(1, 2, 3, 4);
    return output;
}

// CHECK: .y = -
// CHECK: gl_Position
