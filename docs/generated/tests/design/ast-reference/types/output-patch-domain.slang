//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#geometry--and-tessellation-shader-io-types
//META: doc_section_digest=7c904d8992380b74b7c58a3b82c04f1512a6f31a7f5dec239acb5b75fe6f68d7
//META: purpose=An HLSLOutputPatchType is the AST type for `OutputPatch<T,N>`; the HLSL backend preserves the surface spelling at a domain-shader entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLOutputPatchType` is the `HLSLPatchType` leaf for the user
// spelling `OutputPatch<T, N>`. Observable via HLSL emit at a domain
// entry point: the `OutputPatch<...>` keyword survives lowering and
// the parameter type carries the patch's element type and count.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage domain -entry domainMain

struct HSOutput
{
    float4 Pos : SV_POSITION;
};

struct DSOutput
{
    float4 Pos : SV_POSITION;
};

struct PatchConstants
{
    float Edges[3] : SV_TessFactor;
    float Inside : SV_InsideTessFactor;
};

//CHECK-DAG: OutputPatch
//CHECK-DAG: domainMain
[domain("tri")]
DSOutput domainMain(
    PatchConstants pc,
    float3 bary : SV_DomainLocation,
    const OutputPatch<HSOutput, 3> patch)
{
    DSOutput o;
    o.Pos = bary.x * patch[0].Pos
          + bary.y * patch[1].Pos
          + bary.z * patch[2].Pos;
    return o;
}
