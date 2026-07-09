//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=Both a type parameter and a value parameter are inferred implicitly from a fixed-size array argument.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding for type and value" example:
// `sumElements<ElementType : IArithmetic, let N : uint>(ElementType arr[N])`
// is called with an `int[7]` literal, so both ElementType=int and N=7 are
// inferred from the single array argument. CHECK asserts the sum 1..7 == 28,
// proving type and value parameters were inferred together from the array.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func sumElements<ElementType : IArithmetic, let N : uint>(ElementType arr[N]) -> ElementType
{
    ElementType acc = arr[0];
    for (uint i = 1; i < N; ++i)
        acc = acc + arr[i];
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int elements[7] = { 1, 2, 3, 4, 5, 6, 7 };
    outputBuffer[0] = sumElements(elements);
}
// CHECK: 28
