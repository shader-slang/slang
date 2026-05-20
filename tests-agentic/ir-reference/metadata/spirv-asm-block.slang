//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#spir-v-inline-asm
//META: doc_section_digest=83e1d4e4dd9a6f3ff22eeab676848835d3e5cc28439d49da52d478139c73d7a8
//META: purpose=A spirv_asm{...} block lowers to a SPIRVAsm parent opcode that owns SPIRVAsmInst children inside the function body.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the SPIR-V-inline-asm section: `SPIRVAsm` is the parent
// container of an inline-asm block; children are `SPIRVAsmInst`.
// The dump prints them as `SPIRVAsm` block followed by indented
// `SPIRVAsmInst(...)` lines.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int doubled(int x)
{
    return spirv_asm
    {
        OpIMul $$int result $x $x
    };
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = doubled(int(tid.x));
}

// CHECK: SPIRVAsm
// CHECK: SPIRVAsmInst(
