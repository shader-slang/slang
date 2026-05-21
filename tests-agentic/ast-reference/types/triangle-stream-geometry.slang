//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ast-reference/types.md#geometry--and-tessellation-shader-io-types
//META: doc_section_digest=7c904d8992380b74b7c58a3b82c04f1512a6f31a7f5dec239acb5b75fe6f68d7
//META: purpose=An HLSLTriangleStreamType is the AST type for the geometry-shader `TriangleStream<T>` output; the GLSL backend lowers it to per-vertex `EmitVertex()` invocations.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLTriangleStreamType` is the `HLSLStreamOutputType` leaf for
// the geometry-shader user spelling `TriangleStream<T>`. Observable
// via GLSL emit at a geometry entry point: the `.Append(...)`
// method on the stream lowers to `EmitVertex()` per call.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -stage geometry -entry main -profile sm_5_0

struct CoarseVertex
{
    float4 position : POSITION;
};

struct RasterVertex
{
    float4 position : POSITION;
};

//CHECK: void main()
//CHECK: EmitVertex();
[maxvertexcount(3)]
void main(
    triangle CoarseVertex coarse[3],
    inout TriangleStream<RasterVertex> outputStream)
{
    for (int i = 0; i < 3; ++i)
    {
        RasterVertex v;
        v.position = coarse[i].position;
        outputStream.Append(v);
    }
}
