//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=An `optional` conformance constraint enables `if (T is IInteger)` to select behavior at specialization time, using T as IInteger in the then-branch.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Optional type constraint" example: `where optional T :
// IInteger` makes `T is IInteger` a compile-time-resolvable test. For int the
// then-branch (a + b) is chosen; for float the else-branch (a - a = 0). CHECK
// asserts 3 for the int call and 0 for the float call, proving the optional
// constraint drove specialization-time branch selection.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func addTwoIfInts<T>(T a, T b) -> T
    where T : IArithmetic
    where optional T : IInteger
{
    if (T is IInteger)
        return a + b;
    else
        return a - a;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = float(addTwoIfInts(1, 2));
    outputBuffer[1] = addTwoIfInts(1.5f, 2.5f);
}
// CHECK: 3.0
// CHECK: 0.0
