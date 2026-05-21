//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#mesh-shader-geometry-shader-and-per-vertex
//META: doc_section_digest=bc9792e538714c1954b89b0fec254e02a846380cc5a2ee7ae9ffacaf795c351f
//META: purpose=A geometry-shader entry point declaring a `triangle` input parameter lowers to a trianglePrimitiveType decoration on that parameter, with no operands.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's "Mesh shader, geometry shader, and per-vertex" table
// row for `trianglePrimitiveType`: AST origin is the geometry shader
// `triangle` input variant, and the decoration carries no operands.
// Confirm the IR dump emits a bare `[trianglePrimitiveType]` line on
// the geometry-shader's input parameter.

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

// The trianglePrimitiveType decoration attaches to the geometry-shader
// input parameter; the entry-point's nameHint anchors us at user code
// and prevents preamble false-matches.
// CHECK-DAG: [trianglePrimitiveType]
// CHECK-DAG: [nameHint("main")]
