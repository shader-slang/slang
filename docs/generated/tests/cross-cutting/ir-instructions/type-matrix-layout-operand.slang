//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A `float3x3` matrix parameter is typed `Mat(elementType, rowCount, columnCount, layout)` in the IR — exercising the documented four-operand shape of the `matrix` type opcode (with the layout operand explicit).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `matrix` with C++ wrapper `MatrixType` and operand
// shape `(elementType, rowCount, columnCount, layout)`. The existing
// bundle covers the 4x4 size; this test pins down the four-operand
// shape on a smaller (3x3) matrix so the row/column-count axis is
// exercised at a non-power-of-two configuration, and the explicit
// layout operand is observable.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float3x3 m;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = m[0][0];
}

// IR: %m {{.*}}: Mat(Float, 3 : Int, 3 : Int, {{.*}}: Int){{.*}}= global_param
