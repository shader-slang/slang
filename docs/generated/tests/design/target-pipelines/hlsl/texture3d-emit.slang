//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Expansion: a Texture3D `.Load(int4)` call survives HLSL emit with the native Texture3D spelling and `t` register.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Texture3D.Load takes an int4 (xyz + mip) in HLSL. The C-like
// emitter passes this through verbatim with the native type name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int4(0, 0, 0, 0));
}

// CHECK: Texture3D{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: tex_{{[0-9]+}}.Load(int4(
