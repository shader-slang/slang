//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: `bitfieldExtract(a, 0, 32)` (full-width extract on a 32-bit `uint`) lowers to `bitfieldExtract(%a, 0 : UInt, 32 : UInt)` — the upper-edge of the bitfield-width range.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The upper-equal edge: count = width of the operand type. The
// inline payload-encoded count is the documented operand layout.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bitfieldExtract(a, 0, 32);
}

// IR-LABEL: func %main
// IR: bitfieldExtract(%a, 0 : UInt, 32 : UInt)
