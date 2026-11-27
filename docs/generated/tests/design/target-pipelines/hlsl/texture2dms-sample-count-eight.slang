//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=Boundary: a Texture2DMS<float4, 8> survives HLSL emit with the sample count `8` literally embedded in the type spelling.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: sample-count parameter on Texture2DMS. 8 is the
// highest commonly-supported MSAA count; the emit preserves the
// numeric literal verbatim inside the angle-bracketed type.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2DMS<float4, 8> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int2(0, 0), 0);
}

// CHECK: Texture2DMS<float4,{{ *}}8{{ *}}> tex_{{[0-9]+}} : register(t{{[0-9]+}})
