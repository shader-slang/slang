//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=TextureCube.SampleGrad dispatches via OpImageSampleExplicitLod with the Grad image operand on a Cube OpTypeImage.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: SampleGrad on a Cube — gradients are direction
// vectors, not (u,v) deltas. Pins that the Cube OpTypeImage row
// still routes through OpImageSampleExplicitLod with Grad.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

TextureCube<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleGrad(samp,
        float3(1, 0, 0),
        float3(0.01, 0, 0),
        float3(0, 0.01, 0));
}

// CHECK-DAG: OpTypeImage %float Cube
// CHECK-DAG: OpImageSampleExplicitLod {{.*}} Grad
