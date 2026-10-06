//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=18390b63de1003330aaf1c938cc3b9a195ccaebbb584bbb2edb3db5df8f75c02
//META: purpose=A PointStream<T> geometry-shader output parameter surfaces the PointStream opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A PointStream<T> geometry-shader output parameter surfaces the
// PointStream opcode parameterised by its element type in the IR dump.
// A geometry shader's `inout PointStream<RasterVertex>` parameter
// surfaces the `PointStream` opcode parameterised by its element type
// in the IR dump.

//TEST:SIMPLE(filecheck=IR):-profile sm_5_0 -stage geometry -entry main -target hlsl -dump-ir -o -

// SV_Position cannot decorate an array directly; wrap each input vertex
// in a struct that carries the semantic.
struct GsInput
{
    float4 position : SV_Position;
}

struct RasterVertex
{
    float4 position : SV_Position;
}

[maxvertexcount(1)]
void main(point GsInput verts[1], inout PointStream<RasterVertex> outStream)
{
    RasterVertex v;
    v.position = verts[0].position;
    outStream.Append(v);
}

// IR-DAG: PointStream
