//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#diagnostics-during-lowering
//META: doc_section_digest=90444c4cb9fc02bd4dc517a83c3f36f04807193346a8cbe99ca628ff372a49b8
//META: purpose=A chain of 129 nested ordinary calls exhausts the invoke-lowering recursion budget and reports `fatal error[E39997]: maximum type nesting level exceeded` instead of overflowing the native stack.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the invoke-lowering recursion guard is not specific to
// constructor synthesis: the depth counter advances once per nested
// `InvokeExpr`, because a call's arguments are lowered from inside the
// argument-lowering step of the enclosing call. A chain of plain calls to an
// ordinary `int f(int)` is therefore enough to hit it, with no type nesting
// involved at all. The CHECK pins E39997, which proves the compiler reported
// the budget rather than crashing — a stack overflow would produce no
// diagnostic. The depth used here is 129, the first depth that trips the guard
// on this build; 128 still compiles cleanly (see the bundle README's doc-gap
// row on the off-by-one against the documented figure).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int f(int v) { return v + 1; }

[numthreads(1, 1, 1)]
void main()
{
    int x = 0;
    outputBuffer[0] = f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(x)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));
//CHECK: E39997
}
