//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#spirvasmoperand
//META: doc_section_digest=b2230695d92295db8f6572962ce2cbfc6a94232db497c990333151c10edf01d5
//META: purpose=A builtin(...) token in an inline-asm instruction becomes a SPIRVAsmOperandBuiltinVar operand carrying the built-in kind.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the built-in variable reference is carried as a typed operand with
// its kind as an operand of its own. The block loads a subgroup built-in; the
// CHECK requires the operand to print by its opcode mnemonic with a single
// integer operand, which is what lets a later pass materialise the actual
// built-in variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -capability spvGroupNonUniform

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = spirv_asm
    {
        result:$$uint = OpLoad builtin(SubgroupSize:uint)
    };
}

// CHECK: SPIRVAsmInst({{[0-9]+}} : UInt, UInt, result, SPIRVAsmOperandBuiltinVar({{[0-9]+}} : Int))
