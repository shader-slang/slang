//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=The `<<` operator lowers to `shl` and `>>` lowers to `shr`; the doc notes the arithmetic-vs-logical choice is keyed by operand signedness.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two shift operators, on signed and unsigned integers, both
// surface the `shl` / `shr` opcodes the doc lists in the Arithmetic
// and bitwise table.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform uint ua;
uniform int amt;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<uint> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a << amt;
    buf[1] = a >> amt;
    ubuf[0] = ua << amt;
    ubuf[1] = ua >> amt;
}

// IR-LABEL: func %main
// IR: shl(%a, %amt)
// IR: shr(%a, %amt)
// IR: shl(%ua, %amt)
// IR: shr(%ua, %amt)
