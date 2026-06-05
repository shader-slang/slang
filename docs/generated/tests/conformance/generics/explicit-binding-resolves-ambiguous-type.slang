//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Explicit type argument binding `testFunc<IBase>(a, b)` resolves an ambiguous inference (two different non-fundamental struct types) that would otherwise be an error.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Implicit parameter binding, ambiguous type argument).
// Explicit binding `testFunc<IBase>(a, b)` resolves the ambiguity.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IBase {}
struct A : IBase {}
struct B : IBase {}

void testFunc<T>(T x, T y) {}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    A a;
    B b;
    // Explicit binding resolves the ambiguity
    testFunc<IBase>(a, b);
    outputBuffer[0] = 1;  // reached here without error
}
// CHECK: 1
