//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=When inference for a generic type parameter is ambiguous and the candidates are non-fundamental types, specialization fails with an error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding, ambiguous type argument"
// example: `testFunc<T>(T x, T y)` is called with a value of type A and a value
// of type B (both conform to IBase). Inference has two non-fundamental
// candidates for T, which the doc says is an error requiring explicit binding.
// CHECK asserts E39999 "could not specialize generic" fires (non-exhaustive
// skips the trailing "see declaration" note).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

interface IBase
{
}

struct A : IBase
{
}

struct B : IBase
{
}

void testFunc<T>(T x, T y)
{
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a;
    B b;
    testFunc(a, b);
    //CHECK: E39999
    outputBuffer[0] = 0;
}
