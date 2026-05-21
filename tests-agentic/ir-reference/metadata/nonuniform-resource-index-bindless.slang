//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#attr-family
//META: doc_section_digest=f3430c3e5f3d15f08a7b508e5bb6b1ba5106fa40b74c0d99d17989a7e31e58a8
//META: purpose=The NonUniformResourceIndex intrinsic on a bindless Texture2D[] index lowers to a nonUniformResourceIndex IR instruction visible in the IR dump.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Attr-family table: the `nonuniform` Attr opcode originates
// from `NonuniformResourceIndexModifier`. At the dump points
// observable by `-dump-ir` here, the user-facing
// `NonUniformResourceIndex(idx)` surface surfaces as a
// `nonUniformResourceIndex` IR instruction wrapping the index
// operand (see slang-ir-float-non-uniform-resource-index.cpp).
// The bindless `Texture2D[]` array indexing is the natural Slang
// surface that produces this instruction in real shader code.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage fragment

struct Params
{
    Texture2D<float4> textures[16];
    SamplerState samp;
};

ParameterBlock<Params> gParams;

float4 main(
    uint idx : INDEX,
    float2 uv : TEXCOORD0)
    : SV_Target
{
    return gParams.textures[NonUniformResourceIndex(idx)].Sample(gParams.samp, uv);
}

// CHECK: nonUniformResourceIndex
