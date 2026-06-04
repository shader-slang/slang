//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Both the type parameter and the value parameter can be inferred from the call-site argument (array of known element type and known length).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Implicit parameter binding for type and value).
// `ElementType` and `N` are both inferred from the array argument.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

ElementType sumElements<ElementType : IArithmetic, let N : uint>(ElementType arr[N])
{
    ElementType acc = arr[0];
    for (uint i = 1; i < N; ++i)
        acc = acc + arr[i];
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int elements[7] = { 1, 2, 3, 4, 5, 6, 7 };
    // generic parameters ElementType and N are bound implicitly
    outputBuffer[0] = sumElements(elements);
}
// CHECK: 28
