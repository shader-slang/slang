//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=When inference is ambiguous across fundamental scalar types, the element type with the highest promotion rank (int32_t over int8_t) is chosen.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the promotion-rank tie-break rule: `pick<T>` is called with an
// int8_t and an int32_t argument, an ambiguous inference for T. The rule
// selects the higher-rank type int32_t. The body returns `T(300)`; a value of
// 300 is representable in int32_t but wraps to 44 in int8_t, so CHECK asserting
// 300 proves T resolved to the highest-rank fundamental type, not int8_t.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

T pick<T : IArithmetic>(T a, T b)
{
    return T(300);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int8_t a = 3;
    int32_t b = 5;
    outputBuffer[0] = int(pick(a, b));
}
// CHECK: 300
