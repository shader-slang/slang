//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=295e5ac09f4b7f77dd6669103c943586264fcbb5b6e2283e5226b00e48c0393b
//META: purpose=A geometry entry point with inout TriangleStream<T> lowers to a streamOutputTypeLayout opcode in the layout dump.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Layout-family table: `streamOutputTypeLayout` is the
// layout for a geometry-shader stream-output type (variadic,
// min=1, hoistable, synthesized). The natural Slang surface is an
// `inout TriangleStream<T>` (or `LineStream<T>` / `PointStream<T>`)
// parameter on a `[shader("geometry")]` entry point.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage geometry

struct GsIn
{
    float4 position : POSITION;
    float3 color    : COLOR;
};

struct GsOut
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

[shader("geometry")]
[maxvertexcount(3)]
void main(
    triangle GsIn input[3],
    inout TriangleStream<GsOut> stream)
{
    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
    {
        GsOut v;
        v.position = input[i].position;
        v.color = input[i].color;
        stream.Append(v);
    }
}

// CHECK: streamOutputTypeLayout
