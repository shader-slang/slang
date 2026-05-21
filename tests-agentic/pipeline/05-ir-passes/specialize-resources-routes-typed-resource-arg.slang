//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
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
