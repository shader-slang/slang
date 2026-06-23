//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=The basic-scalar `Bool` type appears in the IR for a `bool` parameter — the doc's "`Int`, `Float`, `Bool`, ..." basic-scalar row.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `Bool` as one of the basic scalar type opcodes
// (C++ wrapper `BoolType`). A `uniform bool flag` produces a
// `: Bool` type annotation on the corresponding `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool flag;
uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = flag ? a : 0;
}

// IR: %flag {{.*}}: Bool{{.*}}= global_param
