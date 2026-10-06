//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Defining the same SPIR-V result id twice in one `spirv_asm` block is rejected with the id-redefinition error (E29113).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the `definedIds` bookkeeping in visitSPIRVAsmExpr: the checker
// walks each instruction, works out which operand carries the result id, and
// inserts it into a set of already-defined ids. Re-using `%r` for a second
// `OpConstant` makes that insert fail, and the test pins the resulting
// diagnostic on the *second* definition (the first one stays silent).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry test -stage compute

[numthreads(1, 1, 1)]
void test()
{
    spirv_asm
    {
        %r : $$int = OpConstant $$int 1;
        %r : $$int = OpConstant $$int 2;
//CHECK:             ^^^^^^^^^^ E29113
//CHECK:             ^^^^^^^^^^ SPIRV id '%r' is already defined in the current assembly block
    };
}
