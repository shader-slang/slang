//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Bitwise `~` (unary complement) on `int16_t` lowers to the documented `bitnot` opcode (the 1-operand bit-op family member) with an `Int16` result type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's value-instructions row lists "bit ops" without naming
// the unary-complement opcode. The IR name `bitnot` is the 1-operand
// sibling of `and`/`or`/`xor`. Pins the unary-arity edge of the
// bit-op family.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int16_t a;
uniform RWStructuredBuffer<int16_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = ~a;
}

// IR-LABEL: func %main
// IR: bitnot(%a)
