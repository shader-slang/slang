//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2D paired with SamplerComparisonState dispatches SampleCmpLevelZero via OpImageSampleDrefExplicitLod — the depth-reference comparison form of the sample family.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the comparison-sampler axis. SampleCmpLevelZero
// produces OpImageSampleDrefExplicitLod (Dref = depth reference
// value). The result is a single float, not a float4.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<float> tex;
SamplerComparisonState samp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleCmpLevelZero(samp, float2(0.5, 0.5), 0.5);
}

// CHECK-DAG: OpTypeImage %float 2D
// CHECK-DAG: OpImageSampleDrefExplicitLod
