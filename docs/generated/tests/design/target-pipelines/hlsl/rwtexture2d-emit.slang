//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Expansion: an RWTexture2D is written and read via uint2 subscript; HLSL emit preserves the native RWTexture2D spelling and `u` register.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWTexture2D is the UAV form of Texture2D. HLSL emit preserves
// the type name and binds it to a `u` register.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWTexture2D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[uint2(0, 0)] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[uint2(0, 0)];
}

// CHECK: RWTexture2D{{.*}} tex_{{[0-9]+}} : register(u{{[0-9]+}})
