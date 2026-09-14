//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
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
