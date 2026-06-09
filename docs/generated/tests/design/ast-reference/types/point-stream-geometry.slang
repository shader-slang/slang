//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#geometry--and-tessellation-shader-io-types
//META: doc_section_digest=7c904d8992380b74b7c58a3b82c04f1512a6f31a7f5dec239acb5b75fe6f68d7
//META: purpose=An HLSLPointStreamType is the AST type for the geometry-shader `PointStream<T>` output; the HLSL backend preserves the surface spelling and the stream's per-vertex method emits per the documented stream protocol.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLPointStreamType` is the `HLSLStreamOutputType` leaf for the
// geometry-shader user spelling `PointStream<T>`. Observable via
// HLSL emit at a geometry entry point: the `PointStream<...>`
// type keeps its name and the `.Append(...)` / `.RestartStrip()`
// methods survive.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage geometry -entry main -profile sm_5_0

struct CoarseVertex
{
    float4 position : POSITION;
};

struct RasterVertex
{
    float4 position : POSITION;
};

//CHECK: PointStream
//CHECK: Append
[maxvertexcount(1)]
void main(
    point CoarseVertex coarse[1],
    inout PointStream<RasterVertex> outputStream)
{
    RasterVertex v;
    v.position = coarse[0].position;
    outputStream.Append(v);
}
