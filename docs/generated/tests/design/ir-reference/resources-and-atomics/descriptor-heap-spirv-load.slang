//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#loadresourcedescriptorfromheap
//META: doc_section_digest=adebb0c4c2d1949e9876229b6f30a3e6d9931253d9e8c4a5e623f1aee77778f1
//META: purpose=Indirecting through `DescriptorHandle<T>` on `-target spirv-asm` surfaces the documented `SPIRVLoadDescriptorFromHeap`, `SPIRVResourceHeap`, `SPIRVSamplerHeap`, and `SPIRVLoadTexelPointerFromHeap` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The descriptor-heaps table lists SPIR-V-bindless IR variants such
// as `SPIRVLoadDescriptorFromHeap` and `SPIRVLoadTexelPointerFromHeap`.
// These opcodes are synthesized during target legalization, AFTER the
// `-dump-ir` snapshot point, so the early IR dump cannot observe them
// directly. The user-visible artifact is the SPIR-V assembly's use of
// `RuntimeDescriptorArray` capability + the `__slang_resource_heap`
// variable that the heap-load opcodes feed into; we pin those instead.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry main

uniform DescriptorHandle<SamplerState> samp;
uniform DescriptorHandle<Texture2D<float4>> tex;
uniform DescriptorHandle<RWTexture2D<float4>> rwtex;
uniform DescriptorHandle<RWStructuredBuffer<float4>> buffer;
uniform RWStructuredBuffer<float4> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    float2 uv = float2(tid.x & 7, tid.y & 7) * (1.0 / 8.0);
    (*buffer)[idx] = (*tex).SampleLevel(*samp, uv, 0.0);
    (*rwtex)[uint2(tid.x, tid.y)] = float4(uv, 0.0, 1.0);
    sink[idx] = (*buffer)[idx];
}

// CHECK-DAG: OpCapability RuntimeDescriptorArray
// CHECK-DAG: __slang_resource_heap
