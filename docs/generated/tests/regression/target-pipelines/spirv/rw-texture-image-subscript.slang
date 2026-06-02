//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
