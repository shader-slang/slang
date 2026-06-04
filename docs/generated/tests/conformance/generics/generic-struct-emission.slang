//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A specialized generic struct emits as a monomorphized concrete struct in HLSL; the type parameter is replaced by the concrete type and the struct name is mangled.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// After specialization, the generic struct is lowered to a concrete struct.
// HLSL emission must show a struct definition with the bound element type.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

struct Pair<T>
{
    T first;
    T second;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    Pair<float> p;
    p.first  = 1.0;
    p.second = 2.0;
    outputBuffer[0] = p.first + p.second;
}
// CHECK: struct Pair_{{[a-zA-Z0-9_]+}}
// CHECK: float
