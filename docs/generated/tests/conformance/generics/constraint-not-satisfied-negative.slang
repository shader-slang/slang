//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A call whose argument type does not satisfy the generic's conformance constraint (`where T : IArithmetic`) is rejected at the call site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a conformance constraint is enforced at specialization: a plain
// struct `NotArith` does not conform to IArithmetic, so calling `addTwo<T>`
// (which requires `T : IArithmetic`) with two NotArith values cannot specialize.
// CHECK asserts E39999 "could not specialize generic" fires on the call
// (non-exhaustive skips the trailing "see declaration" note).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

struct NotArith
{
    int x;
}

func addTwo<T>(T a, T b) -> T where T : IArithmetic
{
    return a + b;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    NotArith p = { 1 };
    NotArith q = { 2 };
    addTwo(p, q);
    //CHECK: E39999
    outputBuffer[0] = 0;
}
