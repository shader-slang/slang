//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A vector-typed parameter or value uses the `Vec` IR type with element-type and element-count operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `uint3` parameter `tid` is typed as `Vec(UInt, 3 : Int)` in
// the IR — the doc's "Vector types; hoistable" claim for the
// `vector` opcode taking `(elementType, elementCount)` operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = tid.x;
}

// IR-LABEL: func %main
// IR: param %tid : Vec(UInt, 3 : Int)
