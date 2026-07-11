//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=A generic function with two independent type parameters infers each one separately from its own argument.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that implicit binding infers multiple distinct type parameters
// independently. `firstAsInt<T : IInteger, U : IFloat>(T a, U b)` is called
// with an int and a float, inferring T=int and U=float separately, and returns
// `a.toInt()` using only T's IInteger requirement. CHECK asserts 13, proving
// each parameter bound from its own argument with the correct constraint.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int firstAsInt<T : IInteger, U : IFloat>(T a, U b)
{
    return a.toInt();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = firstAsInt(13, 3.5f);
}
// CHECK: 13
