//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=A `where countof(N) == IntExpr` constraint on a value parameter pack accepts a pack whose length equals the compile-time integer expression.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two parts of the pack-count claim at once: the constrained pack may
// be a *value* pack (`let each N : int`), and the right-hand side may be any
// compile-time integer expression rather than a bare literal (here `1 + 2`).
// The body expands the value pack into an accumulator; CHECK asserts 60
// (10 + 20 + 30), proving the three-element value pack satisfied
// `countof(N) == 1 + 2` and expanded elementwise.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int sumVals<let each N : int>() where countof(N) == 1 + 2
{
    int acc = 0;
    expand acc += each N;
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumVals<10, 20, 30>();
}
// CHECK: 60
