//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#loadresourcedescriptorfromheap
//META: doc_section_digest=51bb78878333a1965bd93f980a078e0c29cd459282a318e33a19fce416018edf
//META: purpose=Indirecting through a `DescriptorHandle<RWStructuredBuffer<T>>` on the HLSL target surfaces the documented `LoadResourceDescriptorFromHeap` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bindless resource-descriptor load. A
// `DescriptorHandle<T>` is dereferenced and the loaded resource is
// used, so the handle cannot be folded away; the CHECK finds the
// opcode in the HLSL-targeted dump where the heap is implicit and the
// index is the only operand.

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
