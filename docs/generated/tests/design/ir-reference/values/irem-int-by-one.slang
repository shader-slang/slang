//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: integer remainder by 1 (`a % 1`) lowers to `irem(%a, 1 : Int)` — the smallest non-zero divisor edge for the documented `irem` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `% 0` is rejected by semantic check; the smallest valid divisor
// is 1. The IR preserves the `irem` opcode with the divisor as
// the inline payload.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a % 1;
}

// IR-LABEL: func %main
// IR: irem(%a, 1 : Int)
