//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2D<uint4> emits OpTypeImage with sampled type %uint instead of %float — the resolveTextureFormat / element-type axis of the image-type table.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: switching the Slang element type from float4 to
// uint4 changes the first operand of OpTypeImage from %float to
// %uint. The dim row stays 2D and the sampled bit stays 1
// (this is a read-only sampled texture, not a storage image).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<uint4> tex;
RWStructuredBuffer<uint4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int3(0, 0, 0));
}

// CHECK: OpTypeImage %uint 2D
