//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=Boundary probe of the documented `matrix` type opcode: a `float4x4` parameter is typed `Mat(Float, 4 : Int, 4 : Int, ...)` in the IR with element-type, row-count, and column-count operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `matrix` alongside `vector` and `Array` as a
// representative type opcode with `(elementType, rowCount, columnCount,
// layout)` operands. A `float4x4` global is enough to pin the operand
// shape down at the documented maximum dimension for HLSL-style
// matrices.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4x4 m;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = m[0][0];
}

// IR: Mat(Float, 4 : Int, 4 : Int{{.*}})
