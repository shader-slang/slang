//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Boundary: Texture2DMS `.Load(int2(0,0), N-1)` with the largest legal sample index for a 4-sample MSAA texture preserves both the int2 coord and the sample-index argument.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: highest legal sample index (N-1 == 3) on a
// 4-sample Texture2DMS Load. HLSL emit preserves the int2 coord
// constructor and the explicit sample index.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2DMS<float4, 4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int2(0, 0), 3);
}

// CHECK: Texture2DMS<float4,{{ *}}4{{ *}}> tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: tex_{{[0-9]+}}.Load(int2(
