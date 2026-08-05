//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=A DescriptorHandleType `DescriptorHandle<T>` is a bindless handle that carries a heap index; the HLSL backend recovers the concrete resource by indexing the descriptor heap with it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a descriptor handle is not an ordinary resource binding
// but an index-carrying handle: the two handle fields live in a
// constant buffer and are used directly as a texture and a sampler.
// CHECK matches the heap subscripts in the emitted HLSL, which is where
// the stored index is turned back into a concrete resource.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

struct Stuff
{
    DescriptorHandle<Texture2D<float4> > tex;
    DescriptorHandle<SamplerState> samp;
}

ConstantBuffer<Stuff> cb;
RWStructuredBuffer<float4> outBuf;

//CHECK-DAG: ResourceDescriptorHeap[
//CHECK-DAG: SamplerDescriptorHeap[
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = cb.tex.SampleLevel(cb.samp, float2(0.5, 0.5), 0.0);
}
