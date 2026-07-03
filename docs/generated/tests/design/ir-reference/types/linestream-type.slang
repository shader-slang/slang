//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=A LineStream<T> geometry-shader output parameter surfaces the LineStream opcode parameterised by its element type in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Resource and texture types" table lists `LineStream`
// (C++ wrapper `HLSLLineStreamType`) with operand
// `elementType: IRType`. A geometry shader's
// `inout LineStream<RasterVertex>` parameter surfaces the
// `LineStream` opcode parameterised by its element type in the
// IR dump.

//TEST:SIMPLE(filecheck=IR):-profile sm_5_0 -stage geometry -entry main -target hlsl -dump-ir -o /dev/null

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

[maxvertexcount(2)]
void main(line GsInput verts[2], inout LineStream<RasterVertex> outStream)
{
    RasterVertex v;
    v.position = verts[0].position;
    outStream.Append(v);
}

// IR-DAG: LineStream
