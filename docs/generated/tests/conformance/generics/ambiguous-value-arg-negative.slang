//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=When inference yields multiple options for a generic value argument (two arrays of different lengths bound to the same `let N`) it is a compile-time error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that conflicting inferences for a generic value parameter are a compile-time error.
// Two arrays of different lengths (7 and 6) are passed where both bind to `let N : uint`;
// the ambiguity cannot be resolved, so CHECK asserts E39999 is emitted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

uint len<let N : uint>(int[N] arr, int[N] arr2)
{
    return N;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int arr1[7] = { };
    int arr2[6] = { };
    outputBuffer[0] = len(arr1, arr2);
//CHECK: E39999
}
