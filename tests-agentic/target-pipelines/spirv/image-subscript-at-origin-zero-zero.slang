//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=An RWTexture2D subscript at the (0,0) origin still lowers to OpImageWrite / OpImageRead - the legalization is shape-driven, not value-driven.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: lower edge of the 2D image coordinate space. The
// legalizeImageSubscript pass converts img[uint2(0,0)] = v into
// OpImageWrite at the origin; the read back lowers to
// OpImageRead. Out-of-bounds behavior is GPU-defined and
// not asserted here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture2D<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint2(0, 0)] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[uint2(0, 0)];
}

// CHECK-DAG: OpImageWrite
// CHECK-DAG: OpImageRead
