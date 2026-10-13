//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `builtin(Name:Type)` operand inside a `spirv_asm` block lowers to a synthesized Input variable decorated with that SPIR-V BuiltIn.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `BuiltinVar` operand case of `visitSPIRVAsmExpr`: the operand's
// known builtin enum value and its parsed Slang type are lowered together into
// a `spirvAsmOperandBuiltinVar`. The user never declares the variable, so the
// only way `OpDecorate ... BuiltIn WorkgroupId` and an Input-storage-class
// OpVariable can appear in the assembly is if that operand created one; the
// OpLoad in the entry point then reads it.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

uint3 getWorkgroupId()
{
    return spirv_asm
    {
        result:$$uint3 = OpLoad builtin(WorkgroupId:uint3);
    };
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = getWorkgroupId().x;
}

//CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn WorkgroupId
//CHECK-DAG: %{{[A-Za-z0-9_]+}} = OpVariable %{{[A-Za-z0-9_]+}} Input
//CHECK-DAG: %{{[A-Za-z0-9_]+}} = OpLoad %v3uint %{{[A-Za-z0-9_]+}}
