//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#context-predicates-and-capability-gates
//META: doc_section_digest=73709830ebcd44a38a75b30d5beb840d2872e0fc7ffd0f1fe4015e73dc477268
//META: purpose=`DescriptorHandle<T>` is the user-facing construct behind the `descriptor_handle` capability gate, which on HLSL can only be turned on by a shader model 6.6 or newer profile; at `cs_6_6` the handle field is a plain `uint2` and its uses become `ResourceDescriptorHeap` / `SamplerDescriptorHeap` subscripts.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Slang surface that reaches the `descriptor_handle`
// capability gate, and what it leaves in the HLSL text. Two
// observations carry the claim: no opaque handle type survives to
// emit — the constant-buffer fields are lowered all the way to
// `uint2` indices — and the texture and sampler are recovered by
// indexing the two SM 6.6 descriptor heaps by name. The profile is
// supplied as `-profile cs_6_6` rather than `-stage compute` so the
// shader model, not just the stage, is what the gate sees.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -profile cs_6_6

struct Params
{
    DescriptorHandle<Texture2D<float4>> tex;
    DescriptorHandle<SamplerState> samp;
};
ConstantBuffer<Params> gParams;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gParams.tex.SampleLevel(gParams.samp, float2(0.5, 0.5), 0.0);
}

// The handles are lowered to plain integer heap indices in the cbuffer struct.
// CHECK: struct Params_{{[0-9]+}}
// CHECK: uint2 tex_{{[0-9]+}};
// CHECK: uint2 samp_{{[0-9]+}};
// Each use recovers the resource from the matching descriptor heap.
// CHECK: Texture2D<float4 >(ResourceDescriptorHeap[
// CHECK: SamplerState(SamplerDescriptorHeap[
