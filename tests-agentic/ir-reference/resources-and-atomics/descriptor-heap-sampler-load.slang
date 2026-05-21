//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#loadresourcedescriptorfromheap
//META: doc_section_digest=adebb0c4c2d1949e9876229b6f30a3e6d9931253d9e8c4a5e623f1aee77778f1
//META: purpose=Indirecting through a `DescriptorHandle<SamplerState>` on the HLSL target surfaces the documented `LoadSamplerDescriptorFromHeap` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's descriptor-heaps row says `LoadSamplerDescriptorFromHeap(index)`
// is the bindless sampler-load counterpart. The Slang surface is the
// `DescriptorHandle<SamplerState>` whose default lowering on HLSL
// emits `__loadSamplerDescriptorFromHeap`.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main

uniform DescriptorHandle<SamplerState> samp;
uniform DescriptorHandle<Texture2D<float4>> tex;
uniform RWStructuredBuffer<float4> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    float2 uv = float2(tid.x & 7, tid.y & 7) * (1.0 / 8.0);
    sink[idx] = (*tex).SampleLevel(*samp, uv, 0.0);
}

// IR-DAG: LoadResourceDescriptorFromHeap
// IR-DAG: LoadSamplerDescriptorFromHeap
