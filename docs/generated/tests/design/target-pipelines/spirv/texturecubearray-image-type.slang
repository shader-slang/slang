//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=TextureCubeArray lowers to OpTypeImage with dim Cube and the Arrayed bit set (1), sampled via a 4-component vector (xyz + layer).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: TextureCubeArray combines two image-type bits:
// dim=Cube AND Arrayed=1. The fifth integer operand (0) is the
// MS bit, distinguishing it from a multi-sampled cube image.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

TextureCubeArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float4(1, 0, 0, 0), 0.0);
}

// CHECK: OpTypeImage %float Cube 2 1 0 1
