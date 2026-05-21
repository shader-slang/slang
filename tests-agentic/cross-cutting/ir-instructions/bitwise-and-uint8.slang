//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Bitwise `&` on `uint8_t` operands lowers to the documented `and` opcode with a `UInt8` result type — unsigned-narrow probe of the bit-op family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `bitwise-and-int8.slang`: the same `and` opcode on the
// unsigned 8-bit element edge. Confirms the IR opcode does not vary
// with signedness while the result type does.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint8_t a;
uniform uint8_t b;
uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a & b;
}

// IR-LABEL: func %main
// IR: and(%a, %b)
