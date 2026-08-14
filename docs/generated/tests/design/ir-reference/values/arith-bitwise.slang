//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Bitwise `&` / `|` / `^` lower to `and` / `or` / `xor`, and prefix `~` lowers to `bitnot`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Arithmetic and bitwise table lists four bitwise opcodes the
// cross-cutting bundle does not cover. A single function combines
// all four so the IR dump surfaces each.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform uint b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a & b;
    buf[1] = a | b;
    buf[2] = a ^ b;
    buf[3] = ~a;
}

// IR-LABEL: func %main
// IR: and(%a, %b)
// IR: or(%a, %b)
// IR: xor(%a, %b)
// IR: bitnot(%a)
