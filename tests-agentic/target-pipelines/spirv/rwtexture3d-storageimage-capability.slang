//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=A RWTexture3D<float4> with no resolved Image Format (Unknown) requires both StorageImageReadWithoutFormat and StorageImageWriteWithoutFormat capabilities.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: resolveTextureFormat leaves the format as Unknown
// for a RWTexture<floatN> when no explicit [[vk::image_format(...)]]
// is given. SPIR-V then requires the two "without format"
// capabilities for reads and writes against the storage image.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture3D<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint3(0, 0, 0)] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[uint3(0, 0, 0)];
}

// CHECK-DAG: OpCapability StorageImageReadWithoutFormat
// CHECK-DAG: OpCapability StorageImageWriteWithoutFormat
