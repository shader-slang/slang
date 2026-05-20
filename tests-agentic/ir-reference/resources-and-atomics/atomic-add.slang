//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<T>::add(v)` lowers to `atomicAdd(%ptr, %val, %order)`, the IR encoding of the atomic add primitive.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's atomic-operations table maps the atomic-add intrinsic to
// `atomicAdd(%ptr, %val)`. The portable Slang surface is the
// `Atomic<T>::add(v)` method on an `Atomic<int>` element of a
// `RWStructuredBuffer`. A trailing memory-order operand encodes the
// `IRMemoryOrder` enum (`0 : Int` is relaxed).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].add(idx + 1);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, {{.*}})
