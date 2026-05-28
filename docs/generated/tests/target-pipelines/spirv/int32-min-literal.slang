//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=The int32 MIN literal (-2147483648, expressed as -2147483647-1) emits as OpConstant %int -2147483648.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the lower edge of the int32 range. We write the
// literal as (-2147483647 - 1) since 2147483648 would not fit
// in a positive int literal. The SPIR-V emit folds back to a
// single OpConstant %int -2147483648.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = -2147483647 - 1;
}

// CHECK: %int_n2147483648 = OpConstant %int -2147483648
// CHECK: OpStore %{{.*}} %int_n2147483648
