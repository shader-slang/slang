//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Left-shift `<<` on an `int64_t` lowers to the documented `shl` opcode with an `Int64` result type — upper-edge probe of the shift sibling of the bit-op family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `bitwise-shl-int8.slang` at the upper edge of the
// integer-width axis: the `shl` opcode applies on a 64-bit signed
// left-operand without changing its name, only the result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t a;
uniform int b;
uniform RWStructuredBuffer<int64_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a << b;
}

// IR-LABEL: func %main
// IR: shl(%a, %{{.*}})
