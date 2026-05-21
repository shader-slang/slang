//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=floatNonUniformResourceIndex runs for Metal with NonUniformResourceIndexFloatMode::Textual: the NonUniformResourceIndex(...) intrinsic is a pass-through on Metal — there is no Metal equivalent of SPIR-V's NonUniform decoration, so the indexing expression survives directly into the texture-array subscript in the emitted MSL.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `floatNonUniformResourceIndex` fires for Metal because the gate
// is `!isSPIRV(target)`. The Textual float mode leaves the index
// expression intact: there is no NonUniform decoration to attach
// in MSL. The natural surface is an array of textures indexed
// per-fragment with a non-uniform index. We verify the indexed
// texture sample reaches the emitted MSL and that no SPIR-V
// `NonUniformResourceIndex` symbol leaks into Metal text.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage fragment

struct VOut
{
    float4 pos : SV_Position;
    float3 uv : UV;
};

Texture2D textures[8];
SamplerState samp;

[shader("fragment")]
float4 main(VOut vin) : SV_Target
{
    int idx = int(vin.uv.z);
    return textures[NonUniformResourceIndex(idx)].Sample(samp, vin.uv.xy);
}

// CHECK: #include <metal_stdlib>
// CHECK: fragment{{.*}} main_0
// Metal's emitted indexing expression preserves the index value:
// CHECK: .sample(
// CHECK-NOT: NonUniformResourceIndex
// CHECK-NOT: NonUniform
