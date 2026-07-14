//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Expansion: an RWTexture1D resource is written and read via the subscript operator; HLSL emit retains the RWTexture1D type and binds it to a `u` register.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWTexture1D is the UAV form of Texture1D. HLSL emit preserves
// the type spelling and chooses the `u` register class because
// the resource is read-write.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWTexture1D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[0] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[0];
}

// CHECK: RWTexture1D{{.*}} tex_{{[0-9]+}} : register(u{{[0-9]+}})
