//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#shape-of-the-emitted-file
//META: doc_section_digest=cd40f48a68f8c4cc01781946c78837919b2702a1a94124525d0a69aadc68ec8c
//META: purpose=The `RasterizerOrdered` access prefix of the HLSL texture-type composition survives emit as `RasterizerOrderedTexture2D<T>`, and the resource binds in the unordered-access `u` register class.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one row of the access-prefix enumeration that the HLSL
// texture-type name is composed from. The bundle already pins the
// `RW` prefix (rwtexture1d/2d/3d) and the bare no-prefix shapes;
// `RasterizerOrdered` is a separate prefix and, unlike a plain
// `Texture2D`, it is a UAV. The CHECKs assert both halves at once:
// the composed type name, and the `u` register class the
// register-semantic mapping assigns it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RasterizerOrderedTexture2D<float4> rov;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = rov[tid.xy];
}

// CHECK: RasterizerOrderedTexture2D<float4 > rov_{{[0-9]+}} : register(u{{[0-9]+}});
