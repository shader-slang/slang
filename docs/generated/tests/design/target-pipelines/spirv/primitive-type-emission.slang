//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=float4 lowers to OpTypeVector %float 4 and int / uint to OpTypeInt 32 (1|0) in SPIR-V emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V's primitive type declarations: OpTypeFloat 32 for
// float, OpTypeInt 32 1 for int, OpTypeInt 32 0 for uint, and
// OpTypeVector %float 4 for float4. All four appear in any
// non-trivial Slang compute shader.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float4(float(tid.x), 1.0, 2.0, 3.0);
}

// CHECK-DAG: %uint = OpTypeInt 32 0
// CHECK-DAG: %float = OpTypeFloat 32
// CHECK-DAG: %v4float = OpTypeVector %float 4
