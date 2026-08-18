//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
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
