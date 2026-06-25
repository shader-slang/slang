//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-06-24T09:20:31Z
//META: source_commit=b25e06aeb8862d207541c3d812c57cb9e381b802
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=When inference of a generic type argument is ambiguous and the types are non-fundamental (two unrelated struct types both conforming to a base), it is a compile-time error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Implicit parameter binding, ambiguous type argument).
// When inferred type is ambiguous and non-fundamental, explicit binding must be used.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IBase {}

struct A : IBase {}
struct B : IBase {}

void testFunc<T>(T x, T y) {}

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    A a;
    B b;
    testFunc(a, b);
//CHECK: E30442
}
