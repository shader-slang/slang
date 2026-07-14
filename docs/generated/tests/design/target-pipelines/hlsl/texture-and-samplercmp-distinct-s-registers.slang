//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Boundary: a SamplerState and a SamplerComparisonState in the same module each take their own `s` register.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: two sampler-class objects of different kinds in
// the same module. Both bind to the `s` register class but with
// distinct indices.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float> tex;
SamplerState samp;
SamplerComparisonState sampCmp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a = tex.SampleLevel(samp, float2(0.5, 0.5), 0.0);
    float b = tex.SampleCmp(sampCmp, float2(0.5, 0.5), 0.5);
    outBuf[tid.x] = a + b;
}

// CHECK-DAG: SamplerState samp_{{[0-9]+}} : register(s{{[0-9]+}})
// CHECK-DAG: SamplerComparisonState sampCmp_{{[0-9]+}} : register(s{{[0-9]+}})
