//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#generics
//META: doc_section_digest=de256a399516188e43bb96d93ea3c68437fd40807db23a0da48c05221ab85f6d
//META: purpose=A generic value parameter `let N : int` can be used at compile time to size arrays or bound loops.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `let N : int` generic value parameter is usable as a compile-time array size and loop bound.
// `sumArray<3>` sums a 3-element array using N in the loop; 6 confirms the
// value parameter resolved correctly to size the iteration.

//TEST:INTERPRET(filecheck=CHECK):

float sumArray<let N : int>(float arr[N])
{
    float s = 0.0;
    for (int i = 0; i < N; i++) s += arr[i];
    return s;
}

void main()
{
    float arr[3] = { 1.0, 2.0, 3.0 };
    // CHECK: 6
    printf("%g\n", sumArray(arr));
}
