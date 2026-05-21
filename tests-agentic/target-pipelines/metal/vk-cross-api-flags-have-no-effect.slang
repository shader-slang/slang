//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=legalizeUniformBufferLoad / invertYOfPositionOutput / rcpWOfPositionInput are filtered out for Metal (gated on isKhronosTarget || HLSL): Vulkan cross-API flags do not mutate Metal MSL emission. Even with -fvk-invert-y and -fvk-use-dx-position-w set, the emitted SV_Position output is unchanged and no `(-y)` rewrite or `1/w` reciprocal appears.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc enumerates `legalizeUniformBufferLoad`,
// `invertYOfPositionOutput`, and `rcpWOfPositionInput` as
// Khronos/HLSL-gated passes that are filtered out for Metal. Their
// absence is observable: even passing the Vulkan cross-API flags
// `-fvk-invert-y` and `-fvk-use-dx-position-w`, the emitted MSL
// preserves the `SV_Position` output verbatim — no Y-flip and no
// `1.0 / w` reciprocal rewrite are introduced.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage vertex -fvk-invert-y -fvk-use-dx-position-w

struct VOut
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VOut main(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), 2.0, 3.0, 4.0);
    return o;
}

// CHECK: #include <metal_stdlib>
// CHECK: vertex{{.*}} main_0
// The position is passed through unchanged: no Y inversion, no W reciprocal.
// CHECK-NOT: pos{{.*}}= {{.*}}-{{.*}}.y
// CHECK-NOT: 1.0{{.*}}/{{.*}}.w
