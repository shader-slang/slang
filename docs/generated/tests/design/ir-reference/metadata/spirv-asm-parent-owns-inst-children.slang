//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#spir-v-inline-asm
//META: doc_section_digest=ce88f8a50b24a7084d3ffc7d6b519378c907043ce6579a0868a664482dac2a56
//META: purpose=A spirv_asm block lowers to a SPIRVAsm parent instruction owning SPIRVAsmInst children, each of which takes its SPIR-V opcode as operand 0.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both the container relationship and the opcode-first operand rule.
// The block contains a single `OpIMul`, so the CHECK first pins the typed
// parent and then requires an indented child whose leading operand is the
// SPIR-V opcode number for that instruction rather than one of its arguments.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int doubled(int x)
{
    return spirv_asm { OpIMul $$int result $x $x };
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = doubled(int(tid.x));
}

// CHECK: SPIRVAsm %{{[a-zA-Z0-9_]+}}
// OpIMul is SPIR-V opcode 132; it occupies the instruction's operand 0.
// CHECK: SPIRVAsmInst(132 : UInt,
