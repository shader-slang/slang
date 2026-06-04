//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Boundary: a TextureCube<float> (scalar element, not float4) keeps its element-type parameter in the HLSL emit angle brackets.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: texture element type (scalar `float` instead of
// the more usual `float4`). HLSL emit carries the angle-bracketed
// element parameter through verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

TextureCube<float> tex;
SamplerState samp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(0.0, 1.0, 0.0), 0.0);
}

// CHECK: TextureCube<float{{ *}}> tex_{{[0-9]+}} : register(t{{[0-9]+}})
