//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A `where T : IArithmetic` conformance constraint lets the generic body call the interface operations (here `+`) on values of type T.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a conformance constraint `T : IArithmetic` makes T implement
// all of IArithmetic's requirements inside the body, so `a + b` type-checks
// before specialization. The function is instantiated for both int and float;
// CHECK asserts each specialization computes its own sum, proving the
// constraint admitted the arithmetic requirement for both concrete types.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func addTwo<T>(T a, T b) -> T where T : IArithmetic
{
    return a + b;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = float(addTwo(3, 4));
    outputBuffer[1] = addTwo(1.5f, 2.25f);
}
// CHECK: 7.0
// CHECK: 3.75
