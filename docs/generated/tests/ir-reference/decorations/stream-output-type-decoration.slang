//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#mesh-shader-geometry-shader-and-per-vertex
//META: doc_section_digest=bc9792e538714c1954b89b0fec254e02a846380cc5a2ee7ae9ffacaf795c351f
//META: purpose=A geometry-shader output declaration using TriangleStream lowers to a streamOutputTypeDecoration recording the HLSL stream-output type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's "Mesh shader, geometry shader, and per-vertex" table
// row for `streamOutputTypeDecoration`: AST origin is a geometry-shader
// output declaration, and the decoration carries a single
// `streamType : IRHLSLStreamOutputType` operand. Confirm the IR dump
// emits the decoration on the geometry-shader entry-point parameter
// declared with `TriangleStream<...>`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage geometry

uniform RWStructuredBuffer<float4> buf;

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : SV_Position;
}

[maxvertexcount(3)]
void main(
    triangle CoarseVertex coarseVertices[3],
    inout TriangleStream<RasterVertex> outputStream,
    uint primitiveID : SV_PrimitiveID)
{
    RasterVertex v;
    v.position = coarseVertices[0].position;
    outputStream.Append(v);
    buf[primitiveID] = v.position;
}

// The streamOutputTypeDecoration carries the HLSL stream-output type
// as an inst reference (the IRHLSLStreamOutputType operand prints as
// %N). Wildcard the operand text since the inst id is unstable.
// CHECK-DAG: [streamOutputTypeDecoration(
// CHECK-DAG: [nameHint("main")]
