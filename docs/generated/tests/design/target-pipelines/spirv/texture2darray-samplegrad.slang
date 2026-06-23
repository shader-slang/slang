//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2DArray.SampleGrad dispatches via OpImageSampleExplicitLod with Grad on a 2D Arrayed=1 OpTypeImage; gradients are float2 because the layer axis is integer.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: SampleGrad on an arrayed image. The layer is the
// integer 3rd coord component; gradients are still float2 (no
// derivative w.r.t. layer). Pins the Arrayed + Grad combination.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2DArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleGrad(samp,
        float3(0.5, 0.5, 0),
        float2(0.1, 0),
        float2(0, 0.1));
}

// CHECK-DAG: OpTypeImage %float 2D 2 1
// CHECK-DAG: OpImageSampleExplicitLod {{.*}} Grad
