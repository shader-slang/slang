//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Bitwise `&` on `uint64_t` operands lowers to the documented `and` opcode with a `UInt64` result type — 64-bit unsigned probe of the bit-op family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `and` opcode at the upper edge of the integer-width axis
// on the unsigned side. Pairs with `bitwise-xor-int64.slang` (signed
// edge of the same axis).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint64_t a;
uniform uint64_t b;
uniform RWStructuredBuffer<uint64_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a & b;
}

// IR-LABEL: func %main
// IR: and(%a, %b)
