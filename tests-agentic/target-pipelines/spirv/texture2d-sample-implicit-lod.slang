//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2D.Sample (no explicit Lod) in a fragment shader dispatches via OpImageSampleImplicitLod — the third sibling of the SampleLevel/SampleGrad family.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Sample (no Lod, no Grad) needs implicit derivatives
// from neighboring fragments, so it must run in a fragment stage,
// not compute. Pins the OpImageSampleImplicitLod opcode.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

Texture2D<float4> tex;
SamplerState samp;

[shader("fragment")]
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
    return tex.Sample(samp, uv);
}

// CHECK-DAG: OpTypeImage %float 2D
// CHECK-DAG: OpImageSampleImplicitLod
