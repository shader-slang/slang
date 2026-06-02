//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Bitwise `&` on `int8_t` operands lowers to the documented `and` arithmetic/logic opcode with an `Int8` result type — narrow-integer probe of the bit-op family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names "arithmetic/logic/comparison/bit ops" in the value-
// instructions row but does not enumerate the bit-width axis. The
// `and` opcode appearing with an `Int8` result type pins the lower
// edge of the integer-width axis for the bit-op family.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t a;
uniform int8_t b;
uniform RWStructuredBuffer<int8_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a & b;
}

// IR-LABEL: func %main
// IR: and(%a, %b)
