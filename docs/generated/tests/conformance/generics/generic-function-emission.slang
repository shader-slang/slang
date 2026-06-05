//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A specialized generic function emits as a concrete monomorphized function in HLSL with the bound type replacing the type parameter.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// After specialization the generic `identity<T>` function is lowered to
// a concrete function. HLSL should show `float` (or the specialized type) in place of T.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

float identity_f(float x) { return x; }

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[0] = identity_f(3.14);
}
// CHECK: float
// CHECK: computeMain
