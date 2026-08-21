//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
//META: purpose=RWTexture2D subscript writes lower via legalizeImageSubscript to OpImageWrite and subscript reads to OpImageRead.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeImageSubscript rewrites HLSL-style tex[uv] = value
// patterns into SPIR-V image opcodes (OpImageWrite for stores,
// OpImageRead for reads). The pass is in the
// Khronos/Metal/GLSL/SPIR-V arm.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture2D<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint2(tid.x, tid.y)] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[uint2(tid.x, tid.y)];
}

// CHECK-DAG: OpImageWrite
// CHECK-DAG: OpImageRead
