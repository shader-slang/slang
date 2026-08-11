//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
//META: purpose=Expansion: the specialize-resources pass routes a resource-typed argument (RWStructuredBuffer<int>) into a concrete specialization — the helper inlines and the buffer access appears in the entry-point body.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `slang-ir-specialize-resources.cpp` specializes parameters whose
// type is a resource. A helper that takes an `RWStructuredBuffer<int>`
// by-value parameter is specialized at the call site so the helper
// inlines away and the buffer subscript appears directly in `main`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> outBuf;
uniform int seed;

[ForceInline]
void writeOne(RWStructuredBuffer<int> dst, int v) { dst[0] = v; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    writeOne(outBuf, seed + int(tid.x));
}

// HLSL: void main
// HLSL-NOT: writeOne
