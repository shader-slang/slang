//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A fixed-size array type appears in the IR as `Array(elementType, elementCount)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `float[4]` field uses the `Array` type opcode that the doc
// lists under "Type instructions" — the operands are
// `(elementType, elementCount)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Bag { float data[4]; }

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Bag b;
    b.data[0] = a;
    b.data[1] = a + 1;
    b.data[2] = a + 2;
    b.data[3] = a + 3;
    buf[0] = b.data[2];
}

// IR-LABEL: struct %Bag
// IR: field(%{{.*}}, Array(Float, 4 : Int))
