//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=d592afa9b9efaf43740554b05a3fdf5b6f6e5077
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=A countof(...) pack-count constraint whose left-side pack is declared by an enclosing generic rather than the current one is rejected with E30431.
//META: intent=negative
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The spec says the left-side `countof(...)` operand must be "a direct
// reference to a generic type pack or value pack parameter declared in the
// current generic declaration", and gives this exact pair as the contrast:
// `countof(TIndex) == countof(D)` is legal inside `inner`, but the reversed
// spelling is not, "because the left-side pack `D` is not declared by
// `inner`". This test pins the rejected direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

struct Outer<let each D : int>
{
    void inner<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(D) == countof(TIndex)
    //CHECK: E30431
    {}
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
