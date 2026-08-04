//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=A generic function's type parameter is inferred implicitly from the call-site argument expression type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies implicit binding: `identity<T>(T x)` is called as `identity(42)`
// with no angle-bracket argument, so T is inferred as int from the literal's
// type. The kernel stores the returned value; CHECK asserts 42 round-trips,
// which is only possible if inference bound T=int and specialized the call.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

T identity<T>(T x)
{
    return x;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = identity(42);
}
// CHECK: 42
