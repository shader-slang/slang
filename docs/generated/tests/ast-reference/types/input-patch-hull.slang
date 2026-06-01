//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#geometry--and-tessellation-shader-io-types
//META: doc_section_digest=7c904d8992380b74b7c58a3b82c04f1512a6f31a7f5dec239acb5b75fe6f68d7
//META: purpose=An HLSLInputPatchType is the AST type for `InputPatch<T,N>`; the HLSL backend preserves the surface spelling at a hull-shader entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLInputPatchType` is the `HLSLPatchType` leaf for the user
// spelling `InputPatch<T, N>`. Observable via HLSL emit at a hull
// entry point: the `InputPatch<...>` keyword survives lowering and
// indexing yields the documented per-control-point value type.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage hull -entry hullMain

struct VSOutput
{
    float4 Pos : SV_POSITION;
};

struct HSOutput
{
    float4 Pos : SV_POSITION;
};

struct PatchConstants
{
    float Edges[3] : SV_TessFactor;
    float Inside : SV_InsideTessFactor;
};

PatchConstants ConstantsHS()
{
    PatchConstants pc;
    pc.Edges[0] = 1.0;
    pc.Edges[1] = 1.0;
    pc.Edges[2] = 1.0;
    pc.Inside = 1.0;
    return pc;
}

//CHECK-DAG: InputPatch
//CHECK-DAG: hullMain
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HSOutput hullMain(InputPatch<VSOutput, 3> patch, uint id : SV_OutputControlPointID)
{
    HSOutput o;
    o.Pos = patch[id].Pos;
    return o;
}
