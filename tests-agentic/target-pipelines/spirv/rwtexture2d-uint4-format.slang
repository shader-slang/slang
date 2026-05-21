//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture2D<uint4> emits OpTypeImage with sampled type %uint and image format Rgba32ui (resolveTextureFormat sets the format from the Slang element type).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the sampled-type axis of OpTypeImage. Switching the
// Slang element type from float4 to uint4 flips the SPIR-V sampled
// type from %float to %uint AND populates the Image Format field
// (Rgba32ui) instead of Unknown. resolveTextureFormat (Phase C
// pass #2) is what fills that format slot for storage images.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture2D<uint4> img;
RWStructuredBuffer<uint4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint2(tid.x, tid.y)] = uint4(1, 2, 3, 4);
    outBuf[tid.x] = img[uint2(tid.x, tid.y)];
}

// CHECK: OpTypeImage %uint 2D {{[0-9]+}} 0 0 2 Rgba32ui
