//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: arithmetic right-shift by 31 (`a >> 31`) on a 32-bit signed int lowers to `shr(%a, 31 : Int)` — the upper-edge valid amount; canonical "extract sign bit" idiom.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The high-edge valid shift amount for a 32-bit signed value.
// On signed operands the doc notes the choice between arithmetic
// and logical right-shift is keyed by operand signedness — the
// signed input means an arithmetic shift, but the IR opcode is
// still the same `shr`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a >> 31;
}

// IR-LABEL: func %main
// IR: shr(%a, 31 : Int)
