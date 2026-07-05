//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=06568c8014909b1a1fe61ae1dd314428312f177b6412e33a4e643d370a1fc6d9
//META: purpose=A generic value parameter with a default value (`let N : int = 3`) can be specialized without an explicit value argument.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the description claim that a value parameter may carry a default via
// `= init-expr`. `struct Arr<let N : int = 3>` is used as `Arr<>`, so N falls
// back to 3 and the field is a 3-element array. CHECK asserts the sum of the
// three elements (3) is stored, proving the default value bound N to 3.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Arr<let N : int = 3>
{
    int v[N];
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Arr<> a;
    a.v[0] = 1;
    a.v[1] = 1;
    a.v[2] = 1;
    outputBuffer[0] = a.v[0] + a.v[1] + a.v[2];
}
// CHECK: 3
