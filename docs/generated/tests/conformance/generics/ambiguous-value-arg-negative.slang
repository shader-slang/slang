//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=When inference yields conflicting values for one generic value parameter (two arrays of different lengths bound to the same `let N`), specialization fails with an error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding, ambiguous value argument"
// example: `len<let N : uint>(int[N], int[N])` receives an int[7] and an int[6],
// so N cannot be inferred consistently. The compiler rejects the call; CHECK
// asserts E39999 "could not specialize generic" fires on the call site
// (non-exhaustive skips the trailing "see declaration" note).

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
    //CHECK: E39999
}
