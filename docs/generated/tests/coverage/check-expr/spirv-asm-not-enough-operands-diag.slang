//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A `spirv_asm` instruction with fewer operands than its result-id position needs is rejected with the not-enough-operands error (E29112).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the result-id registration step of visitSPIRVAsmExpr: the
// checker asks the SPIR-V grammar which operand index holds the result id, and
// when the written instruction has fewer operands than that index it cannot
// register anything. `OpLoad` has a result id but is written here with no
// operands at all, which is the shortest input that reaches that branch.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry test -stage compute

[numthreads(1, 1, 1)]
void test()
{
    spirv_asm { OpLoad; };
//CHECK:        ^^^^^^ E29112
//CHECK:        ^^^^^^ not enough operands for OpLoad
}
