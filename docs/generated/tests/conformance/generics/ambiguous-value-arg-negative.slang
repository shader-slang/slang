//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#implicit-parameter-binding-ambiguous-value-argument
//META: doc_section_digest=d91f482916124f53d789e942e45a31fc7f6d8c8621157c05540b759a5798b3e4
//META: purpose=When inference yields conflicting values for one generic value parameter (two arrays of different lengths bound to the same `let N`), specialization fails with an error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding, ambiguous value argument"
// example: `len<let N : uint>(int[N], int[N])` receives an int[7] and an int[6],
// so N cannot be inferred consistently. The compiler rejects the call; CHECK
// asserts E30442 "cannot deduce generic argument" fires on the call site with
// the conflicting 7/6 requirements (non-exhaustive skips the
// trailing "see declaration" note).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

uint len<let N : uint>(int[N] arr, int[N] arr2)
{
    return N;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int arr1[7] = {};
    int arr2[6] = {};
    outputBuffer[0] = len(arr1, arr2);
//CHECK:                 ^ cannot deduce generic argument for 'N': conflicting requirements '7' and '6'
    //CHECK: E30442
}
