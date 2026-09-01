//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=An unrecognised SPIR-V enumerator name in a `spirv_asm` operand is reported as an unknown SPIR-V identifier (E29107).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the NamedValue operand resolution ladder in visitSPIRVAsmExpr:
// a bare identifier operand is looked up first as an enumerator for the
// instruction's operand kind, then as a type-prefixed enumerator, then as an
// opcode, and (for OpExtInst) as a GLSL.std.450 name. When every lookup misses,
// checking fails with E29107. The test feeds a name that exists in none of
// those tables and pins that end-of-ladder diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry test -stage compute

[numthreads(1, 1, 1)]
void test()
{
    spirv_asm { OpCapability NotARealCapability; };
//CHECK:                     ^^^^^^^^^^^^^^^^^^ E29107
//CHECK:                     ^^^^^^^^^^^^^^^^^^ unknown SPIR-V identifier NotARealCapability, it's not a known enumerator or opcode
}
