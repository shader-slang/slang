//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=Reading and writing a groupshared array element produces OpAccessChain against a _ptr_Workgroup pointer, exercising the Workgroup storage class on both the load and store sides.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the bundle pins that `groupshared` lowers to the
// Workgroup storage class. This test pins the load/store side: an
// indexed read and write through a Workgroup pointer emits
// OpAccessChain %_ptr_Workgroup_int.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

groupshared int shared_data[64];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint gtid : SV_GroupIndex)
{
    shared_data[gtid] = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = shared_data[(gtid + 1) % 64];
}

// CHECK-DAG: %_ptr_Workgroup_int = OpTypePointer Workgroup %int
// CHECK-DAG: OpAccessChain %_ptr_Workgroup_int %shared_data
