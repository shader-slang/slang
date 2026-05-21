//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
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
