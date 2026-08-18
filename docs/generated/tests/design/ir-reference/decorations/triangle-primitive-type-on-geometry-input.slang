//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#mesh-shader-geometry-shader-and-per-vertex
//META: doc_section_digest=2ff1e06875bdf0287adb28f870622eecec1458e3d8f28891804e092f6b9f6938
//META: purpose=A geometry-shader entry point declaring a `triangle` input parameter lowers to a trianglePrimitiveType decoration on that parameter, with no operands.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the geometry-shader input primitive kind is recorded as a
// nullary decoration selected by the modifier keyword. The CHECK-DAG lines
// pair the bare `[trianglePrimitiveType]` with the entry point's `nameHint`
// so the match is anchored at user code rather than in the core-module
// preamble.

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
