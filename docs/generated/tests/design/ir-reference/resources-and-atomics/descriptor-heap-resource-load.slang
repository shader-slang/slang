//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#loadresourcedescriptorfromheap
//META: doc_section_digest=adebb0c4c2d1949e9876229b6f30a3e6d9931253d9e8c4a5e623f1aee77778f1
//META: purpose=Indirecting through a `DescriptorHandle<RWStructuredBuffer<T>>` on the HLSL target surfaces the documented `LoadResourceDescriptorFromHeap` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's descriptor-heaps row notes `LoadResourceDescriptorFromHeap(index)`
// is the IR form of indexing into a bindless resource heap. The portable
// Slang surface is a `DescriptorHandle<T>` whose default `getDescriptorFromHandle`
// path lowers to `__loadResourceDescriptorFromHeap(...)` on the HLSL backend.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main

uniform DescriptorHandle<RWStructuredBuffer<float4>> buffer;
uniform DescriptorHandle<StructuredBuffer<float4>> src;
uniform RWStructuredBuffer<float4> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    (*buffer)[idx] = (*src)[idx];
    sink[idx] = (*buffer)[idx];
}

// IR: LoadResourceDescriptorFromHeap
