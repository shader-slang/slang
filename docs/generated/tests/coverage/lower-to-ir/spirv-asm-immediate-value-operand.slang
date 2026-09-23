//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `!expr` immediate operand inside a `spirv_asm` block lowers the Slang constant as a literal enum operand rather than as an id.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `SlangImmediateValue` operand case of `visitSPIRVAsmExpr`. A `!`
// operand lowers its expression as an r-value and wraps it with
// `spirvAsmOperandEnum`, so the value is written into the instruction word
// itself instead of becoming a `%id` reference. Here the GLSL.std.450 opcode
// number is supplied as a module-scope Slang constant (31 = Sqrt); if the
// operand had been lowered as an ordinary value the assembler would have
// emitted a constant id, whereas the disassembly shows the extended
// instruction's literal selector resolved to the `Sqrt` mnemonic.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

static const uint kSqrtOp = 31;

float mySqrt(float x)
{
    return spirv_asm
    {
        result:$$float = OpExtInst glsl450 !kSqrtOp $x;
    };
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = mySqrt(float(tid.x) + 16.0f);
}

//CHECK-DAG: OpExtInstImport "GLSL.std.450"
//CHECK-DAG: %{{[A-Za-z0-9_]+}} = OpExtInst %float %{{[A-Za-z0-9_]+}} Sqrt %{{[A-Za-z0-9_]+}}
