//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=c70214a3d38c8f47b31dbc3940b51ba5fcdeb6d61435076d7a051fbb6e018f86
//META: purpose=`MaxVertexCountAttribute` (`[maxvertexcount(N)]`) on a geometry entry point appears in emitted HLSL with the documented count.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `MaxVertexCountAttribute` carries the maximum vertex count a
// geometry shader emits. The emitted HLSL for a geometry entry point
// must reproduce the chosen count as `[maxvertexcount(N)]`.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_5_0 -stage geometry -entry main

struct GsIn
{
    float4 pos : SV_Position;
};

struct GsOut
{
    float4 pos : SV_Position;
};

[maxvertexcount(3)]
void main(triangle GsIn input[3], inout TriangleStream<GsOut> outStream)
{
    for (int i = 0; i < 3; ++i)
    {
        GsOut o;
        o.pos = input[i].pos;
        outStream.Append(o);
    }
}

//CHECK: maxvertexcount(3)
