//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Right-shift `>>` on a `uint64_t` lowers to the documented `shr` opcode with a `UInt64` result type — upper-edge of the integer-width axis for the shift sibling of the bit-op family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `shr` is the right-shift IR opcode. A 64-bit unsigned left-operand
// pins the upper edge of the integer-width axis for shifts. The
// signedness controls logical-vs-arithmetic shift; the opcode name
// itself is shared.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint64_t a;
uniform int b;
uniform RWStructuredBuffer<uint64_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a >> b;
}

// IR-LABEL: func %main
// IR: shr(%a, %{{.*}})
