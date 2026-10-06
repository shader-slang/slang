//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#implicit-parameter-binding-ambiguous-type-argument
//META: doc_section_digest=5a242a0b2f3a0ce246085c303baec4847733bd0badeaee717cbb03060d2a52a9
//META: purpose=When inference for a generic type parameter is ambiguous and the candidates are non-fundamental types, specialization fails with an error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding, ambiguous type argument"
// example: `testFunc<T>(T x, T y)` is called with a value of type A and a value
// of type B (both conform to IBase). Inference has two non-fundamental
// candidates for T, which the doc says is an error requiring explicit binding.
// CHECK asserts E30442 "cannot deduce generic argument" fires with the
// conflicting-requirements detail (non-exhaustive skips the trailing
// "see declaration" note).

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
//CHECK:    ^ cannot deduce generic argument for 'T': conflicting requirements 'A' and 'B'
    //CHECK: E30442
    outputBuffer[0] = 0;
}
