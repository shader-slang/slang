//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=06568c8014909b1a1fe61ae1dd314428312f177b6412e33a4e643d370a1fc6d9
//META: purpose=A generic value parameter may be of Boolean type and drive compile-time branch selection.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the description claim that a generic value parameter's type may be
// Boolean. `sel<let flag : bool>()` branches on the compile-time flag. Two
// specializations `sel<true>` and `sel<false>` are called; CHECK asserts 100
// then 200, proving a bool value parameter is accepted and specialized to each
// literal.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int sel<let flag : bool>()
{
    if (flag)
        return 100;
    else
        return 200;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sel<true>();
    outputBuffer[1] = sel<false>();
}
// CHECK: 100
// CHECK: 200
