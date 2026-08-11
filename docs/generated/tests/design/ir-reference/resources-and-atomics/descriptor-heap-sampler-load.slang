//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#loadresourcedescriptorfromheap
//META: doc_section_digest=51bb78878333a1965bd93f980a078e0c29cd459282a318e33a19fce416018edf
//META: purpose=Indirecting through a `DescriptorHandle<SamplerState>` on the HLSL target surfaces the documented `LoadSamplerDescriptorFromHeap` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a sampler descriptor handle and a resource descriptor
// handle load through two different opcodes. Both handles are used in
// the same shader and the CHECKs are order-independent, so the test
// asserts the presence of each without depending on which handle the
// lowering visits first.

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
