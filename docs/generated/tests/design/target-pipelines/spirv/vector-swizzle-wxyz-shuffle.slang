//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A full-width float4 swizzle (.wxyz) on a loaded buffer value emits an OpVectorShuffle reorder with indices 3 0 1 2.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a full-width 4-element swizzle that is not the
// identity (so the SCCP passes cannot collapse it). The data
// flow goes through a runtime-loaded vector to defeat
// constant folding.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;
StructuredBuffer<float4> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 v = inBuf[tid.x];
    outBuf[tid.x] = v.wxyz;
}

// CHECK: OpVectorShuffle %v4float {{[%_a-zA-Z0-9]+}} {{[%_a-zA-Z0-9]+}} 3 0 1 2
