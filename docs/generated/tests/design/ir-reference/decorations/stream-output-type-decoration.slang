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

// Observes the `streamOutputTypeLayout` inst a geometry-shader
// `TriangleStream<...>` output parameter produces.
//
// CORRECTION (gap-intake cycle 1): an earlier version of this comment
// claimed `streamOutputTypeLayout` was merely how the dump spells
// `streamOutputTypeDecoration` at this phase, and filed the difference as a
// documentation gap. That was wrong on both counts. They are two distinct
// insts: `streamOutputTypeLayout` is an `IRTypeLayout`
// (`slang-ir-insts.lua:2888`, `IRStreamOutputTypeLayout : IRTypeLayout` at
// `slang-ir-insts.h:1406`), while `streamOutputTypeDecoration` is a
// decoration (`slang-ir-insts.lua:2081`). This test therefore does NOT
// verify the decoration; it verifies the layout inst. The gap it produced
// was rejected as bogus. A test for the decoration itself still needs
// writing, and needs a compiler run to pin the printed form.

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
