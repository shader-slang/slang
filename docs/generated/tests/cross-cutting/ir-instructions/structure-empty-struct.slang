//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=Boundary probe of the `struct` parent opcode at the empty-children edge: an empty `struct {}` still lowers to a `struct` IR opcode with no `field` children.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc describes `struct` as a "parent container" whose children
// are `field` rows. The lower edge of the field-count axis is zero —
// an empty struct must still produce a `struct` IR opcode for the
// type declaration even though it has no `field` children.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Empty {}

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Empty e;
    buf[0] = 1;
}

// IR: struct %Empty
