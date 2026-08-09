//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#untypedresourcehandle-and-untypedsamplerhandle
//META: doc_section_digest=75ee899053793b3043363438c88abda61127a16198c9bd25ae6ca84b91201696
//META: purpose=Indexing SamplerDescriptorHeap yields a value of the nullary IR type UntypedSamplerHandle, distinct from the resource-side handle type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The sampler descriptor heap has a counterpart handle type of its own,
// so the two heaps are not interchangeable at the IR level. A compute
// kernel fetches a SamplerState out of SamplerDescriptorHeap with a
// runtime index and samples a texture with it so nothing is dead-coded;
// the IR dump is FileChecked inside `func %main` for a `let` typed
// UntypedSamplerHandle followed by the SamplerState-typed local, so the
// sampler-side handle is what the heap index converts through.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

uniform uint heapIndex;
Texture2D<float4> tex;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    SamplerState samp = SamplerDescriptorHeap[heapIndex];
    outputBuffer[0] = tex.SampleLevel(samp, float2(0.5, 0.5), 0);
}

// IR: func %main
// IR: : UntypedSamplerHandle{{.*}}= call
// IR: let %samp{{.*}}: SamplerState
