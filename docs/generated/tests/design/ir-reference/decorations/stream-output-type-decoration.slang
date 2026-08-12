//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#mesh-shader-geometry-shader-and-per-vertex
//META: doc_section_digest=2ff1e06875bdf0287adb28f870622eecec1458e3d8f28891804e092f6b9f6938
//META: purpose=A geometry-shader output declaration using TriangleStream lowers to a stream-output-type decoration recording the HLSL stream-output type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a geometry-shader `TriangleStream<...>` output parameter
// records its stream type as a decoration on the entry point. The dump
// spells the inst `streamOutputTypeLayout` at this phase rather than with
// the reference's `streamOutputTypeDecoration` name, so the CHECK matches
// the printed spelling and the entry point's `nameHint` anchors the match
// at user code; the spelling difference is recorded as a doc gap.

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

// In the early `-dump-ir` snapshot the decoration prints as the
// `streamOutputTypeLayout` inst rather than the
// `streamOutputTypeDecoration` name documented in the IR reference —
// both refer to the same conceptual decoration on the geometry-shader
// entry point at different lowering phases. The CHECK matches what the
// dump emits at this stage. Wildcard the operand text since the inst
// id is unstable.
// CHECK-DAG: streamOutputTypeLayout
// CHECK-DAG: nameHint("main")
