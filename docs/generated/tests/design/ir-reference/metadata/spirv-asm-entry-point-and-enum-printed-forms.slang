//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#spirvasmoperand
//META: doc_section_digest=30d7b2afee044e4800553745578314d872096bca3b9b7d00e70fdc652dd523c8
//META: purpose=The __entryPoint token prints bare with no parentheses inside its SPIRVAsmInst, while a named enumerator beside it prints as only its wrapped integer.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two of the printed-form rules that decide how much of an inline-asm
// operand's kind survives into a dump. `__entryPoint` is nullary and falls
// through to the generic printer, which for a zero-operand instruction emits
// the mnemonic with no parentheses at all; the execution-mode name next to it
// is an enumerator whose kind is erased, leaving only the wrapped integer. One
// `SPIRVAsmInst` line therefore shows both rules at once, and the CHECK pins
// the whole operand list so a kind that started naming itself would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<uint> gOut;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    spirv_asm
    {
        OpExecutionMode __entryPoint DerivativeGroupQuadsNV;
    };
    gOut[0] = tid.x;
}

// Operand 0 is the SPIR-V opcode number for OpExecutionMode; `__entryPoint`
// follows it with no parentheses, and the execution-mode enumerator has been
// reduced to a bare integer with nothing naming its operand kind.
// CHECK: SPIRVAsmInst(16 : UInt, __entryPoint, {{[0-9]+}} : UInt)
