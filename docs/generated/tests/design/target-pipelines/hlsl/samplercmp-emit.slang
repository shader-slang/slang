//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=Expansion: a SamplerComparisonState binds to an `s` register and `tex.SampleCmp(sampCmp, uv, ref)` survives HLSL emit verbatim.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SamplerComparisonState is the depth-compare sampler variant.
// HLSL emit preserves the SamplerComparisonState type name and
// the SampleCmp call form.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float> tex;
SamplerComparisonState sampCmp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleCmp(sampCmp, float2(0.5, 0.5), 0.5);
}

// CHECK: SamplerComparisonState sampCmp_{{[0-9]+}} : register(s{{[0-9]+}})
// CHECK: SampleCmp(
