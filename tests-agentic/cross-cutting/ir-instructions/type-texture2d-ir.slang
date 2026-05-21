//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A `Texture2D<float4>` parameter is typed with the documented `Texture` type opcode in the IR, carrying its (elementType, shape, isArray, isMS, sampleCount, access, ...) operand row.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `Texture` under "Type instructions" with operand
// shape `(elementType, shape, isArray, isMS, sampleCount, access,
// isShadow, isCombined, format)`. A `Texture2D<float4>` produces an
// IR type tagged `TextureType(Vec(Float, 4 : Int), TextureShape2DType,
// ...)` — the element type is `float4`, the shape is the 2D-specific
// shape opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2D<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = tex.SampleLevel(samp, float2(0,0), 0);
}

// IR: %tex {{.*}}: TextureType(Vec(Float, 4 : Int), TextureShape2DType,{{.*}})
