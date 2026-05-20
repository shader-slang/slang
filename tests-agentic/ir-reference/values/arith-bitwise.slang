//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
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
