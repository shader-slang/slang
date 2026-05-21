//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=`>` on two `uint64_t` operands lowers to the documented `cmpGT` opcode with a `Bool` result — 64-bit unsigned probe of the comparison family on its already-tested `cmpGT` member.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing bundle covers `cmpGT` only at `int` width
// (`comparison-cmpgt-ir.slang`). A 64-bit unsigned probe pins the
// same opcode at the upper edge of the integer-width axis and on the
// unsigned side, where signedness controls the underlying SPIR-V
// emit instruction but the IR opcode stays the same.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint64_t a;
uniform uint64_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = (a > b) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: cmpGT(%a, %b)
